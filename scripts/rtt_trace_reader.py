#!/usr/bin/env python3
"""
FreeRTOS Trace Reader for RTT

This script reads FreeRTOS trace data via RTT using either J-Link or OpenOCD.
It supports both SEGGER J-Link and ST-Link (via OpenOCD) for the STM32F205.

Usage:
    # Using J-Link
    python3 rtt_trace_reader.py -d STM32F205RB -p jlink

    # Using OpenOCD with ST-Link
    python3 rtt_trace_reader.py -d stm32f2x -p openocd

    # Save to file
    python3 rtt_trace_reader.py -d STM32F205RB -o trace.bin
"""

import argparse
import sys
import time
from pathlib import Path
from typing import Optional

from rtt_reader import JLinkRttReader as SharedJLinkRttReader
from rtt_reader import OpenOcdRttReader as SharedOpenOcdRttReader

# Trace event types (must match C header)
TRACE_EVENTS = {
    0x01: "TASK_SWITCHED_IN",
    0x02: "TASK_SWITCHED_OUT",
    0x03: "TASK_CREATE",
    0x04: "TASK_DELETE",
    0x05: "TASK_READY",
    0x06: "TASK_SUSPENDED",
    0x07: "TASK_RESUMED",
    0x10: "ISR_ENTER",
    0x11: "ISR_EXIT",
    0x20: "QUEUE_CREATE",
    0x21: "QUEUE_SEND",
    0x22: "QUEUE_RECEIVE",
    0x30: "SEMAPHORE_CREATE",
    0x31: "SEMAPHORE_GIVE",
    0x32: "SEMAPHORE_TAKE",
    0x40: "MUTEX_CREATE",
    0x41: "MUTEX_GIVE",
    0x42: "MUTEX_TAKE",
    0x50: "TIMER_CREATE",
    0x51: "TIMER_START",
    0x52: "TIMER_STOP",
    0x60: "MALLOC",
    0x61: "FREE",
}


class JLinkRttReader(SharedJLinkRttReader):
    """RTT reader using pylink for J-Link"""

    def __init__(self, device: str, channel: int = 1):
        super().__init__(device=device)
        self.channel = channel

    def read(self) -> Optional[bytes]:
        """Read data from RTT channel"""
        return self.read_rtt(self.channel)


class OpenOcdRttReader(SharedOpenOcdRttReader):
    """Trace-channel adapter for the shared OpenOCD RTT backend."""

    def __init__(self, device: str, channel: int = 1, host: str = "localhost", port: int = 4444, rtt_port: int = 9090):
        super().__init__(host=host, port=port, rtt_port=rtt_port)
        self.device = device
        self.channel = channel

    @property
    def telnet(self):
        """Compatibility alias for the former command connection attribute."""
        return self.socket

    def read(self) -> Optional[bytes]:
        """Read data from RTT channel"""
        return self.read_rtt(self.channel)


class TraceReader:
    """Main trace reader class"""

    def __init__(self, probe: str, device: str, channel: int = 1, output_file: Optional[Path] = None):
        self.probe = probe
        self.channel = channel
        self.output_file = output_file
        self.running = False

        # Initialize appropriate reader
        if probe == "jlink":
            self.reader = JLinkRttReader(device, channel)
        elif probe == "openocd":
            self.reader = OpenOcdRttReader(device, channel)
        else:
            raise ValueError(f"Unknown probe type: {probe}")

    def run(self):
        """Main reading loop"""
        if not self.reader.connect():
            return 1

        self.running = True
        print(f"Reading trace data from RTT channel {self.channel}")
        print("Press Ctrl+C to stop\n")

        output_fh = None
        if self.output_file:
            output_fh = open(self.output_file, "wb")
            print(f"Saving trace to {self.output_file}")

        try:
            while self.running:
                data = self.reader.read()
                if data and len(data) > 0:
                    # Write to file if specified
                    if output_fh:
                        output_fh.write(data)
                        output_fh.flush()

                    # Display data
                    try:
                        text = data.decode("utf-8", errors="ignore")
                        if text:
                            print(text, end="", flush=True)
                    except Exception:
                        # Binary data - show hex
                        print(f"[Binary: {len(data)} bytes]")
                else:
                    time.sleep(0.01)

        except KeyboardInterrupt:
            print("\n\nStopped by user")
        finally:
            if output_fh:
                output_fh.close()
            self.reader.disconnect()

        return 0


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="FreeRTOS Trace Reader - Read trace data via RTT (J-Link or OpenOCD)",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Read from J-Link
  %(prog)s -d STM32F205RB -p jlink

  # Read from OpenOCD (ST-Link)
  %(prog)s -d stm32f2x -p openocd

  # Save to file
  %(prog)s -d STM32F205RB -p jlink -o trace.bin
        """,
    )

    parser.add_argument("-d", "--device", required=True, help="Target device (e.g., STM32F205RB for J-Link, stm32f2x for OpenOCD)")
    parser.add_argument("-p", "--probe", choices=["jlink", "openocd"], default="jlink", help="Debug probe type (default: jlink)")
    parser.add_argument("-c", "--channel", type=int, default=1, help="RTT channel number for trace data (default: 1)")
    parser.add_argument("-o", "--output", type=Path, help="Output file to save trace data")

    args = parser.parse_args()

    reader = TraceReader(probe=args.probe, device=args.device, channel=args.channel, output_file=args.output)

    return reader.run()


if __name__ == "__main__":
    sys.exit(main())
