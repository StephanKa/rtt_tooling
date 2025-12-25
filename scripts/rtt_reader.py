#!/usr/bin/env python3
"""
RTT Reader - Host-side RTT reading tool

This script provides host-side RTT reading capabilities compatible with:
- OpenOCD (via telnet interface)
- SEGGER J-Link (via pylink library)

It can read RTT output from embedded devices running unit tests or applications.
"""

import argparse
import socket
import sys
import time
from abc import ABC, abstractmethod
from enum import Enum
from typing import Optional


class RttBackend(Enum):
    """RTT backend types"""

    OPENOCD = "openocd"
    JLINK = "jlink"


class RttReader(ABC):
    """Abstract base class for RTT readers"""

    @abstractmethod
    def connect(self) -> bool:
        """Connect to the target device"""

    @abstractmethod
    def disconnect(self) -> None:
        """Disconnect from the target device"""

    @abstractmethod
    def read_rtt(self, channel: int = 0, size: int = 1024) -> Optional[bytes]:
        """Read data from RTT channel"""

    @abstractmethod
    def is_connected(self) -> bool:
        """Check if connected to target"""


class OpenOcdRttReader(RttReader):
    """RTT reader using OpenOCD telnet interface"""

    def __init__(self, host: str = "localhost", port: int = 4444):
        """
        Initialize OpenOCD RTT reader

        Args:
            host: OpenOCD telnet host
            port: OpenOCD telnet port
        """
        self.host = host
        self.port = port
        self.socket: Optional[socket.socket] = None
        self._connected = False

    def connect(self) -> bool:
        """Connect to OpenOCD telnet interface"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(5.0)
            self.socket.connect((self.host, self.port))

            # Read welcome message
            self.socket.recv(4096)

            # Setup RTT
            self._send_command('rtt setup 0x20000000 0x10000 "SEGGER RTT"')
            time.sleep(0.1)
            self._send_command("rtt start")
            time.sleep(0.1)

            self._connected = True
            print(f"Connected to OpenOCD at {self.host}:{self.port}")
            return True
        except Exception as e:
            print(f"Failed to connect to OpenOCD: {e}", file=sys.stderr)
            self._connected = False
            return False

    def disconnect(self) -> None:
        """Disconnect from OpenOCD"""
        if self.socket:
            try:
                self._send_command("rtt stop")
                self.socket.close()
            except Exception:
                pass
            finally:
                self.socket = None
                self._connected = False

    def _send_command(self, command: str) -> str:
        """Send command to OpenOCD and get response"""
        if not self.socket:
            return ""

        try:
            self.socket.sendall((command + "\n").encode())
            time.sleep(0.05)
            return self.socket.recv(4096).decode("utf-8", errors="ignore")
        except Exception as e:
            print(f"Command error: {e}", file=sys.stderr)
            return ""

    def read_rtt(self, channel: int = 0, size: int = 1024) -> Optional[bytes]:
        """Read data from RTT channel via OpenOCD"""
        if not self._connected or not self.socket:
            return None

        try:
            # OpenOCD RTT polling command
            response = self._send_command(f"rtt server {channel}")
            if response:
                # Extract actual RTT data from response
                # OpenOCD returns data in various formats, we need to parse it
                lines = response.split("\n")
                data = []
                for line in lines:
                    # Skip command echo and prompt
                    if line.startswith(">") or "rtt" in line.lower():
                        continue
                    if line.strip():
                        data.append(line)

                if data:
                    return "\n".join(data).encode("utf-8")

            return None
        except Exception as e:
            print(f"Read error: {e}", file=sys.stderr)
            return None

    def is_connected(self) -> bool:
        """Check if connected to OpenOCD"""
        return self._connected


class JLinkRttReader(RttReader):
    """RTT reader using SEGGER J-Link via pylink"""

    def __init__(self, device: str = "STM32F205RB", interface: str = "SWD", speed: int = 4000):
        """
        Initialize J-Link RTT reader

        Args:
            device: Target device name
            interface: Debug interface (SWD or JTAG)
            speed: Interface speed in kHz
        """
        self.device = device
        self.interface = interface
        self.speed = speed
        self.jlink = None
        self._connected = False

        # Try to import pylink
        try:
            import pylink

            self.pylink_module = pylink
        except ImportError:
            self.pylink_module = None
            print("Warning: pylink not installed. Install with: pip install pylink-square", file=sys.stderr)

    def connect(self) -> bool:
        """Connect to J-Link and start RTT"""
        if not self.pylink_module:
            print("Error: pylink module not available", file=sys.stderr)
            return False

        try:
            self.jlink = self.pylink_module.JLink()
            self.jlink.open()

            # Set interface
            if self.interface.upper() == "SWD":
                self.jlink.set_tif(self.pylink_module.enums.JLinkInterfaces.SWD)
            else:
                self.jlink.set_tif(self.pylink_module.enums.JLinkInterfaces.JTAG)

            # Connect to device
            self.jlink.connect(self.device, speed=self.speed)

            # Start RTT
            self.jlink.rtt_start()

            # Wait for RTT to initialize
            time.sleep(0.5)

            self._connected = True
            print(f"Connected to {self.device} via J-Link ({self.interface} @ {self.speed} kHz)")
            return True
        except Exception as e:
            print(f"Failed to connect to J-Link: {e}", file=sys.stderr)
            self._connected = False
            return False

    def disconnect(self) -> None:
        """Disconnect from J-Link"""
        if self.jlink:
            try:
                self.jlink.rtt_stop()
                self.jlink.close()
            except Exception:
                pass
            finally:
                self.jlink = None
                self._connected = False

    def read_rtt(self, channel: int = 0, size: int = 1024) -> Optional[bytes]:
        """Read data from RTT channel via J-Link"""
        if not self._connected or not self.jlink:
            return None

        try:
            data = self.jlink.rtt_read(channel, size)
            if data:
                return bytes(data)
            return None
        except Exception as e:
            print(f"Read error: {e}", file=sys.stderr)
            return None

    def is_connected(self) -> bool:
        """Check if connected to J-Link"""
        return self._connected


class RttReaderApp:
    """Main RTT reader application"""

    def __init__(self, reader: RttReader, output_file: Optional[str] = None):
        """
        Initialize RTT reader application

        Args:
            reader: RTT reader backend
            output_file: Optional file to save output
        """
        self.reader = reader
        self.output_file = output_file
        self.running = False

    def run(self, channel: int = 0, poll_interval: float = 0.01):
        """
        Main reading loop

        Args:
            channel: RTT channel to read from
            poll_interval: Polling interval in seconds
        """
        if not self.reader.connect():
            print("Failed to connect to target", file=sys.stderr)
            return 1

        self.running = True
        print(f"Reading RTT channel {channel}. Press Ctrl+C to exit.\n")

        output_handle = None
        if self.output_file:
            try:
                output_handle = open(self.output_file, "wb")
                print(f"Saving output to {self.output_file}")
            except Exception as e:
                print(f"Failed to open output file: {e}", file=sys.stderr)
                self.reader.disconnect()
                return 1

        try:
            while self.running:
                data = self.reader.read_rtt(channel)
                if data:
                    # Output to console
                    sys.stdout.buffer.write(data)
                    sys.stdout.flush()

                    # Save to file if specified
                    if output_handle:
                        output_handle.write(data)
                        output_handle.flush()
                else:
                    time.sleep(poll_interval)
        except KeyboardInterrupt:
            print("\n\nStopped by user")
        finally:
            if output_handle:
                output_handle.close()
            self.reader.disconnect()

        return 0


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="RTT Reader - Host-side RTT reading for OpenOCD and J-Link",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Read from J-Link
  %(prog)s --backend jlink --device STM32F205RB

  # Read from OpenOCD
  %(prog)s --backend openocd --host localhost --port 4444

  # Save output to file
  %(prog)s --backend jlink --device STM32F205RB --output rtt_log.txt
        """,
    )

    parser.add_argument("-b", "--backend", choices=["openocd", "jlink"], default="openocd", help="RTT backend to use (default: openocd)")

    # OpenOCD options
    openocd_group = parser.add_argument_group("OpenOCD options")
    openocd_group.add_argument("--host", default="localhost", help="OpenOCD telnet host (default: localhost)")
    openocd_group.add_argument("--port", type=int, default=4444, help="OpenOCD telnet port (default: 4444)")

    # J-Link options
    jlink_group = parser.add_argument_group("J-Link options")
    jlink_group.add_argument("-d", "--device", default="STM32F205RB", help="Target device name for J-Link (default: STM32F205RB)")
    jlink_group.add_argument("-i", "--interface", choices=["SWD", "JTAG"], default="SWD", help="Debug interface for J-Link (default: SWD)")
    jlink_group.add_argument("-s", "--speed", type=int, default=4000, help="Interface speed in kHz for J-Link (default: 4000)")

    # Common options
    parser.add_argument("-c", "--channel", type=int, default=0, help="RTT channel number (default: 0)")
    parser.add_argument("-o", "--output", help="Save output to file")
    parser.add_argument("--poll-interval", type=float, default=0.01, help="Polling interval in seconds (default: 0.01)")

    args = parser.parse_args()

    # Create appropriate reader
    if args.backend == "openocd":
        reader = OpenOcdRttReader(host=args.host, port=args.port)
    else:  # jlink
        reader = JLinkRttReader(device=args.device, interface=args.interface, speed=args.speed)

    # Create and run application
    app = RttReaderApp(reader, output_file=args.output)
    return app.run(channel=args.channel, poll_interval=args.poll_interval)


if __name__ == "__main__":
    sys.exit(main())
