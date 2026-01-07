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


class JLinkRttReader:
    """RTT reader using pylink for J-Link"""

    def __init__(self, device: str, channel: int = 1):
        self.device = device
        self.channel = channel
        self.jlink = None

    def connect(self) -> bool:
        """Connect to J-Link"""
        try:
            import pylink

            self.jlink = pylink.JLink()
            self.jlink.open()
            self.jlink.set_tif(pylink.enums.JLinkInterfaces.SWD)
            self.jlink.connect(self.device)
            self.jlink.rtt_start()
            print(f"Connected to {self.device} via J-Link")
            return True
        except ImportError:
            print("Error: pylink not installed. Install with: pip install pylink-square", file=sys.stderr)
            return False
        except Exception as e:
            print(f"Error connecting to J-Link: {e}", file=sys.stderr)
            return False

    def disconnect(self):
        """Disconnect from J-Link"""
        if self.jlink:
            try:
                self.jlink.rtt_stop()
                self.jlink.close()
            except Exception:
                pass

    def read(self) -> Optional[bytes]:
        """Read data from RTT channel"""
        if not self.jlink:
            return None
        try:
            data = self.jlink.rtt_read(self.channel, 1024)
            return bytes(data) if data else None
        except Exception as e:
            print(f"Error reading RTT: {e}", file=sys.stderr)
            return None


class OpenOcdRttReader:
    """RTT reader using OpenOCD telnet interface"""

    def __init__(self, device: str, channel: int = 1, host: str = "localhost", port: int = 19022):
        self.device = device
        self.channel = channel
        self.host = host
        self.port = port
        self.telnet = None
        self.rtt_address = None

    def connect(self) -> bool:
        """Connect to OpenOCD via telnet"""
        # add server start
        # new connection via telnet, grep port
        # read data from new connection
        try:
            # start openocd
            # openocd  --file st_nucleo_l4.cfg
            import telnetlib

            self.telnet = telnetlib.Telnet(self.host, self.port, timeout=5)

            print(f"Connected to OpenOCD at {self.host}:{self.port}")
            return True
        except ImportError:
            print("Error: telnetlib not available", file=sys.stderr)
            return False
        except Exception as e:
            print(f"Error connecting to OpenOCD: {e}", file=sys.stderr)
            print("Make sure OpenOCD is running with: openocd -f interface/stlink.cfg -f target/stm32f2x.cfg")
            return False

    def disconnect(self):
        """Disconnect from OpenOCD"""
        if self.telnet:
            try:
                self._send_command("rtt stop")
                self.telnet.close()
            except Exception:
                pass

    def _send_command(self, cmd: str) -> str:
        """Send command to OpenOCD"""
        if not self.telnet:
            return ""
        self.telnet.write(f"{cmd}\n".encode())
        return self.telnet.read_until(b"> ", timeout=1).decode()

    def read(self) -> Optional[bytes]:
        """Read data from RTT channel"""
        if not self.telnet:
            return None
        try:
            # Use OpenOCD RTT polling
            response = self.telnet.read_until(b"\n")
            if response and len(response) > 0:
                # Filter out command echo and prompt
                return response
            return None
        except Exception as e:
            print(f"Error reading RTT: {e}", file=sys.stderr)
            return None


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
