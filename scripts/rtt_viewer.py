#!/usr/bin/env python3
"""
RTT Logger Viewer

This script connects to a SEGGER J-Link and displays RTT output in real-time.
It can be used for monitoring and debugging embedded applications.
"""

import argparse
import sys
import time
from typing import Optional

from rtt_reader import JLinkRttReader, OpenOcdRttReader, RttReader


class RttViewer:
    """RTT output viewer for SEGGER J-Link"""

    def __init__(
        self,
        device: str = "STM32F205RB",
        interface: str = "SWD",
        speed: int = 4000,
        backend: str = "jlink",
        host: str = "localhost",
        port: int = 4444,
        rtt_port: int = 9090,
    ):
        """
        Initialize RTT viewer

        Args:
            device: Target device name
            interface: Debug interface (SWD or JTAG)
            speed: Interface speed in kHz
        """
        self.device = device
        self.interface = interface
        self.speed = speed
        self.backend = backend
        self.running = False
        if backend == "jlink":
            self.reader: RttReader = JLinkRttReader(device=device, interface=interface, speed=speed)
        elif backend == "openocd":
            self.reader = OpenOcdRttReader(host=host, port=port, rtt_port=rtt_port)
        else:
            raise ValueError(f"Unknown backend: {backend}")

    def connect(self) -> bool:
        """
        Connect to target device

        Returns:
            True if connection successful, False otherwise
        """
        return self.reader.connect()

    def disconnect(self):
        """Disconnect from target device"""
        self.running = False
        self.reader.disconnect()

    def read_rtt(self, channel: int = 0) -> Optional[str]:
        """
        Read data from RTT channel

        Args:
            channel: RTT channel number

        Returns:
            String data if available, None otherwise
        """
        data = self.reader.read_rtt(channel)
        return data.decode("utf-8", errors="replace") if data else None

    def run(self, channel: int = 0):
        """
        Main viewer loop

        Args:
            channel: RTT channel to monitor
        """
        if not self.connect():
            print("Failed to connect to target", file=sys.stderr)
            return

        self.running = True
        print(f"Monitoring RTT channel {channel}. Press Ctrl+C to exit.\n")

        try:
            while self.running:
                data = self.read_rtt(channel)
                if data:
                    print(data, end="", flush=True)
                else:
                    time.sleep(0.01)  # Small delay to prevent busy waiting
        except KeyboardInterrupt:
            print("\n\nStopped by user")
        finally:
            self.disconnect()


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description="RTT Logger Viewer - Monitor SEGGER RTT output")
    parser.add_argument("-b", "--backend", choices=["jlink", "openocd"], default="jlink", help="RTT backend (default: jlink)")
    parser.add_argument("-d", "--device", default="STM32F205RB", help="Target device name (default: STM32F205RB)")
    parser.add_argument("-i", "--interface", choices=["SWD", "JTAG"], default="SWD", help="Debug interface (default: SWD)")
    parser.add_argument("-s", "--speed", type=int, default=4000, help="Interface speed in kHz (default: 4000)")
    parser.add_argument("-c", "--channel", type=int, default=0, help="RTT channel number (default: 0)")
    parser.add_argument("--host", default="localhost", help="OpenOCD host (default: localhost)")
    parser.add_argument("--port", type=int, default=4444, help="OpenOCD command port (default: 4444)")
    parser.add_argument("--rtt-port", type=int, default=9090, help="OpenOCD RTT data port (default: 9090)")

    args = parser.parse_args()

    viewer = RttViewer(
        device=args.device,
        interface=args.interface,
        speed=args.speed,
        backend=args.backend,
        host=args.host,
        port=args.port,
        rtt_port=args.rtt_port,
    )
    viewer.run(channel=args.channel)


if __name__ == "__main__":
    main()
