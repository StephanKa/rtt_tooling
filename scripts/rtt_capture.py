#!/usr/bin/env python3
"""Unified multi-channel SEGGER RTT capture utility."""

import argparse
import json
import sys
import time
from dataclasses import asdict, dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Callable, Dict, List, Optional

from rtt_reader import JLinkRttReader, OpenOcdRttReader, RttReader
from rtt_trace_analyzer import TraceAnalyzer, TraceParser


@dataclass
class ChannelStats:
    """Capture statistics for one RTT up-channel."""

    channel: int
    bytes_captured: int = 0
    chunks_captured: int = 0


class RttCapture:
    """Capture one or more RTT channels into a reproducible session directory."""

    def __init__(
        self,
        backend: str,
        channels: List[int],
        output_dir: Path,
        duration: Optional[float] = None,
        poll_interval: float = 0.01,
        device: str = "STM32F205RB",
        interface: str = "SWD",
        speed: int = 4000,
        host: str = "localhost",
        port: int = 4444,
        rtt_port: int = 9090,
        cpu_frequency: int = 120000000,
        reader_factory: Optional[Callable[[int], RttReader]] = None,
    ):
        if not channels:
            raise ValueError("At least one RTT channel is required")
        if len(set(channels)) != len(channels) or any(channel < 0 for channel in channels):
            raise ValueError("RTT channels must be unique non-negative integers")
        if duration is not None and duration <= 0:
            raise ValueError("Capture duration must be positive")
        if poll_interval < 0:
            raise ValueError("Poll interval cannot be negative")

        self.backend = backend
        self.channels = channels
        self.output_dir = output_dir
        self.duration = duration
        self.poll_interval = poll_interval
        self.device = device
        self.cpu_frequency = cpu_frequency
        self.stats = {channel: ChannelStats(channel) for channel in channels}

        if reader_factory is not None:
            self.readers = {channel: reader_factory(channel) for channel in channels}
        elif backend == "jlink":
            shared_reader = JLinkRttReader(device=device, interface=interface, speed=speed)
            self.readers = {channel: shared_reader for channel in channels}
        elif backend == "openocd":
            self.readers = {
                channel: OpenOcdRttReader(host=host, port=port, rtt_port=rtt_port + index)
                for index, channel in enumerate(channels)
            }
        else:
            raise ValueError(f"Unknown backend: {backend}")

    def capture(self) -> Dict[int, ChannelStats]:
        """Capture until the configured duration expires or Ctrl+C is pressed."""
        self.output_dir.mkdir(parents=True, exist_ok=True)
        unique_readers = list(dict.fromkeys(self.readers.values()))
        connected = []
        files = {}
        started_at = datetime.now(timezone.utc)
        start = time.monotonic()

        try:
            for reader in unique_readers:
                if not reader.connect():
                    raise ConnectionError(f"Failed to connect {self.backend} RTT backend")
                connected.append(reader)

            files = {channel: (self.output_dir / f"channel-{channel}.bin").open("wb") for channel in self.channels}
            while self.duration is None or time.monotonic() - start < self.duration:
                received = False
                for channel in self.channels:
                    data = self.readers[channel].read_rtt(channel, 4096)
                    if not data:
                        continue
                    files[channel].write(data)
                    self.stats[channel].bytes_captured += len(data)
                    self.stats[channel].chunks_captured += 1
                    received = True
                if not received and self.poll_interval:
                    time.sleep(self.poll_interval)
        except KeyboardInterrupt:
            pass
        finally:
            for file_handle in files.values():
                file_handle.close()
            for reader in reversed(connected):
                reader.disconnect()

            elapsed = time.monotonic() - start
            metadata = {
                "schema_version": 1,
                "started_at": started_at.isoformat(),
                "duration_seconds": elapsed,
                "backend": self.backend,
                "device": self.device,
                "cpu_frequency_hz": self.cpu_frequency,
                "channels": [asdict(self.stats[channel]) for channel in self.channels],
            }
            (self.output_dir / "metadata.json").write_text(json.dumps(metadata, indent=2), encoding="utf-8")

        return self.stats

    def export_trace(self, channel: int, output_file: Path) -> bool:
        """Convert a captured trace channel to Perfetto-compatible JSON."""
        if channel not in self.channels:
            raise ValueError(f"Trace channel {channel} was not captured")
        parser = TraceParser(self.output_dir / f"channel-{channel}.bin")
        parser.cpu_frequency = self.cpu_frequency
        if not parser.parse():
            return False
        TraceAnalyzer(parser).export_perfetto(output_file)
        return True


def parse_channels(value: str) -> List[int]:
    """Parse a comma-separated channel list."""
    try:
        channels = [int(item.strip(), 0) for item in value.split(",") if item.strip()]
    except ValueError as error:
        raise argparse.ArgumentTypeError("channels must be comma-separated integers") from error
    if not channels or len(set(channels)) != len(channels) or any(channel < 0 for channel in channels):
        raise argparse.ArgumentTypeError("channels must be unique non-negative integers")
    return channels


def main() -> int:
    """Command-line entry point."""
    parser = argparse.ArgumentParser(description="Capture multiple SEGGER RTT channels with session metadata")
    parser.add_argument("-b", "--backend", choices=["jlink", "openocd"], required=True)
    parser.add_argument("-c", "--channels", type=parse_channels, default=[0], help="Comma-separated RTT channels (default: 0)")
    parser.add_argument("-o", "--output-dir", type=Path, required=True, help="Directory for raw channel files and metadata")
    parser.add_argument("--duration", type=float, help="Capture duration in seconds; omit to run until Ctrl+C")
    parser.add_argument("--poll-interval", type=float, default=0.01, help="Idle polling delay in seconds")
    parser.add_argument("-d", "--device", default="STM32F205RB", help="J-Link target device")
    parser.add_argument("-i", "--interface", choices=["SWD", "JTAG"], default="SWD")
    parser.add_argument("-s", "--speed", type=int, default=4000, help="J-Link speed in kHz")
    parser.add_argument("--host", default="localhost", help="OpenOCD host")
    parser.add_argument("--port", type=int, default=4444, help="OpenOCD command port")
    parser.add_argument("--rtt-port", type=int, default=9090, help="First OpenOCD RTT data port")
    parser.add_argument("--cpu-freq", type=int, default=120000000, help="Target CPU frequency in Hz")
    parser.add_argument("--trace-channel", type=int, help="Captured channel containing FreeRTOS trace frames")
    parser.add_argument("--perfetto", type=Path, help="Perfetto JSON output path for --trace-channel")
    args = parser.parse_args()

    capture = RttCapture(
        backend=args.backend,
        channels=args.channels,
        output_dir=args.output_dir,
        duration=args.duration,
        poll_interval=args.poll_interval,
        device=args.device,
        interface=args.interface,
        speed=args.speed,
        host=args.host,
        port=args.port,
        rtt_port=args.rtt_port,
        cpu_frequency=args.cpu_freq,
    )

    try:
        stats = capture.capture()
        for channel in args.channels:
            channel_stats = stats[channel]
            print(f"Channel {channel}: {channel_stats.bytes_captured} bytes in {channel_stats.chunks_captured} chunks")
        if args.perfetto:
            if args.trace_channel is None:
                parser.error("--perfetto requires --trace-channel")
            if not capture.export_trace(args.trace_channel, args.perfetto):
                return 1
        return 0
    except (ConnectionError, OSError, ValueError) as error:
        print(f"Capture failed: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
