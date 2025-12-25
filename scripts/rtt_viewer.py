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


class RttViewer:
    """RTT output viewer for SEGGER J-Link"""

    def __init__(self, device: str = "STM32F205RB", interface: str = "SWD", speed: int = 4000):
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
        self.running = False

    def connect(self) -> bool:
        """
        Connect to target device

        Returns:
            True if connection successful, False otherwise
        """
        print(f"Connecting to {self.device} via {self.interface} @ {self.speed} kHz...")
        # In a real implementation, this would use pylink or similar
        print("Note: This is a placeholder. Install pylink for actual RTT support.")
        return True

    def disconnect(self):
        """Disconnect from target device"""
        print("Disconnecting...")
        self.running = False

    def read_rtt(self, channel: int = 0) -> Optional[str]:
        """
        Read data from RTT channel

        Args:
            channel: RTT channel number

        Returns:
            String data if available, None otherwise
        """
        # Placeholder implementation
        return None

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
    parser.add_argument("-d", "--device", default="STM32F205RB", help="Target device name (default: STM32F205RB)")
    parser.add_argument("-i", "--interface", choices=["SWD", "JTAG"], default="SWD", help="Debug interface (default: SWD)")
    parser.add_argument("-s", "--speed", type=int, default=4000, help="Interface speed in kHz (default: 4000)")
    parser.add_argument("-c", "--channel", type=int, default=0, help="RTT channel number (default: 0)")

    args = parser.parse_args()

    viewer = RttViewer(device=args.device, interface=args.interface, speed=args.speed)
    viewer.run(channel=args.channel)


if __name__ == "__main__":
    main()
