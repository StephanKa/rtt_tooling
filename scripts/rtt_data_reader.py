#!/usr/bin/env python3
"""
RTT Data Reader - Host-side data reading and formatting tool

This script reads structured data sent via RTT from embedded devices using
the rtt_data C++ library. It decodes the data packets and formats them for
display or further analysis.
"""

import argparse
import struct
import sys
import time
from dataclasses import dataclass
from enum import IntEnum
from typing import Any, Optional, Tuple


class DataType(IntEnum):
    """Data types for RTT data transmission (must match C++ enum)"""

    Int8 = 0
    UInt8 = 1
    Int16 = 2
    UInt16 = 3
    Int32 = 4
    UInt32 = 5
    Int64 = 6
    UInt64 = 7
    Float = 8
    Double = 9
    String = 10
    Binary = 11


@dataclass
class DataHeader:
    """Header for data packets"""

    magic: bytes
    data_type: DataType
    reserved: int
    size: int
    timestamp: int


class RttDataReader:
    """Reader for RTT data packets"""

    MAGIC_BYTES = b"RD"
    HEADER_SIZE = 12  # 2 (magic) + 1 (type) + 1 (reserved) + 4 (size) + 4 (timestamp)
    HEADER_FORMAT = "<2sBBII"  # Little-endian: 2 bytes, byte, byte, uint32, uint32

    # Type format strings for struct.unpack
    TYPE_FORMATS = {
        DataType.Int8: "b",
        DataType.UInt8: "B",
        DataType.Int16: "<h",
        DataType.UInt16: "<H",
        DataType.Int32: "<i",
        DataType.UInt32: "<I",
        DataType.Int64: "<q",
        DataType.UInt64: "<Q",
        DataType.Float: "<f",
        DataType.Double: "<d",
    }

    def __init__(self, verbose: bool = False):
        """
        Initialize RTT data reader

        Args:
            verbose: Enable verbose output
        """
        self.verbose = verbose
        self.packet_count = 0
        self.error_count = 0

    def parse_header(self, data: bytes) -> Optional[DataHeader]:
        """
        Parse data packet header

        Args:
            data: Raw header bytes (at least HEADER_SIZE bytes)

        Returns:
            DataHeader object if valid, None otherwise
        """
        if len(data) < self.HEADER_SIZE:
            return None

        try:
            magic, data_type, reserved, size, timestamp = struct.unpack(self.HEADER_FORMAT, data[: self.HEADER_SIZE])

            # Validate magic bytes
            if magic != self.MAGIC_BYTES:
                if self.verbose:
                    print(f"Invalid magic bytes: {magic.hex()}", file=sys.stderr)
                return None

            # Validate data type
            try:
                data_type = DataType(data_type)
            except ValueError:
                if self.verbose:
                    print(f"Invalid data type: {data_type}", file=sys.stderr)
                return None

            return DataHeader(magic, data_type, reserved, size, timestamp)

        except struct.error as e:
            if self.verbose:
                print(f"Header parsing error: {e}", file=sys.stderr)
            return None

    def parse_data(self, header: DataHeader, data: bytes) -> Optional[Any]:
        """
        Parse data payload based on header information

        Args:
            header: Data header
            data: Raw data bytes

        Returns:
            Parsed data value or None on error
        """
        if len(data) < header.size:
            if self.verbose:
                print(f"Insufficient data: expected {header.size}, got {len(data)}", file=sys.stderr)
            return None

        payload = data[: header.size]

        try:
            # Handle string type
            if header.data_type == DataType.String:
                return payload.decode("utf-8", errors="replace")

            # Handle binary type
            if header.data_type == DataType.Binary:
                return payload.hex()

            # Handle numeric types
            if header.data_type in self.TYPE_FORMATS:
                fmt = self.TYPE_FORMATS[header.data_type]
                return struct.unpack(fmt, payload)[0]

            if self.verbose:
                print(f"Unknown data type: {header.data_type}", file=sys.stderr)
            return None

        except (struct.error, UnicodeDecodeError) as e:
            if self.verbose:
                print(f"Data parsing error: {e}", file=sys.stderr)
            return None

    def format_value(self, header: DataHeader, value: Any) -> str:
        """
        Format a value for display

        Args:
            header: Data header
            value: Parsed value

        Returns:
            Formatted string
        """
        type_name = header.data_type.name

        if header.data_type == DataType.Binary:
            return f"[{type_name}] 0x{value}"
        if header.data_type == DataType.String:
            return f'[{type_name}] "{value}"'
        if header.data_type in [DataType.Float, DataType.Double]:
            return f"[{type_name}] {value:.6f}"
        return f"[{type_name}] {value}"

    def process_packet(self, data: bytes) -> Tuple[Optional[Any], int]:
        """
        Process a complete data packet

        Args:
            data: Raw data containing header and payload

        Returns:
            Tuple of (parsed_value, bytes_consumed)
        """
        # Parse header
        header = self.parse_header(data)
        if header is None:
            self.error_count += 1
            return None, 0

        # Check if we have enough data for payload
        total_size = self.HEADER_SIZE + header.size
        if len(data) < total_size:
            # Not enough data yet
            return None, 0

        # Parse data payload
        payload = data[self.HEADER_SIZE : total_size]
        value = self.parse_data(header, payload)

        if value is not None:
            self.packet_count += 1

            # Format output
            timestamp_str = f"[{header.timestamp:06d}] " if header.timestamp > 0 else ""
            formatted = self.format_value(header, value)
            print(f"{timestamp_str}{formatted}")

            return value, total_size
        self.error_count += 1
        return None, total_size


def read_from_rtt(backend: str, channel: int = 1, **kwargs) -> None:
    """
    Read data from RTT channel

    Args:
        backend: RTT backend ('openocd' or 'jlink')
        channel: RTT channel number
        **kwargs: Backend-specific arguments
    """
    reader = RttDataReader(verbose=kwargs.get("verbose", False))

    # Import appropriate RTT reader
    try:
        if backend == "openocd":
            from rtt_reader import OpenOcdRttReader

            rtt = OpenOcdRttReader(host=kwargs.get("host", "localhost"), port=kwargs.get("port", 4444))
        elif backend == "jlink":
            from rtt_reader import JLinkRttReader

            rtt = JLinkRttReader(device=kwargs.get("device", "STM32F205RB"), interface=kwargs.get("interface", "SWD"))
        else:
            print(f"Unsupported backend: {backend}", file=sys.stderr)
            return
    except ImportError as e:
        print(f"Failed to import RTT reader: {e}", file=sys.stderr)
        print("Make sure rtt_reader.py is in the same directory or PYTHONPATH", file=sys.stderr)
        return

    # Connect to target
    if not rtt.connect():
        print("Failed to connect to target", file=sys.stderr)
        return

    print(f"Reading RTT data from channel {channel}...")
    print("Press Ctrl+C to stop\n")

    buffer = bytearray()

    try:
        while True:
            # Read from RTT
            data = rtt.read_rtt(channel=channel, size=4096)
            if data:
                buffer.extend(data)

                # Process all complete packets in buffer
                while len(buffer) >= RttDataReader.HEADER_SIZE:
                    _, consumed = reader.process_packet(bytes(buffer))

                    if consumed > 0:
                        # Remove processed data from buffer
                        buffer = buffer[consumed:]
                    else:
                        # Not enough data for complete packet, wait for more
                        break

            else:
                # No data, small delay
                time.sleep(0.01)

    except KeyboardInterrupt:
        print(f"\n\nStopped. Packets processed: {reader.packet_count}, Errors: {reader.error_count}")
    finally:
        rtt.disconnect()


def read_from_file(filename: str, verbose: bool = False) -> None:
    """
    Read and parse data from a binary file

    Args:
        filename: Path to binary file
        verbose: Enable verbose output
    """
    reader = RttDataReader(verbose=verbose)

    try:
        with open(filename, "rb") as f:
            data = f.read()

        buffer = bytearray(data)

        print(f"Processing {len(buffer)} bytes from {filename}...\n")

        while len(buffer) >= RttDataReader.HEADER_SIZE:
            _, consumed = reader.process_packet(bytes(buffer))

            if consumed > 0:
                buffer = buffer[consumed:]
            else:
                # Skip one byte and try again
                buffer = buffer[1:]

        print(f"\nPackets processed: {reader.packet_count}, Errors: {reader.error_count}")

    except FileNotFoundError:
        print(f"File not found: {filename}", file=sys.stderr)
    except Exception as e:
        print(f"Error reading file: {e}", file=sys.stderr)


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="RTT Data Reader - Read and format structured RTT data",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Read from OpenOCD
  %(prog)s --backend openocd --host localhost --port 4444 --channel 1

  # Read from J-Link
  %(prog)s --backend jlink --device STM32F205RB --channel 1

  # Parse binary file
  %(prog)s --file data.bin

  # Verbose output
  %(prog)s --backend openocd --verbose
        """,
    )

    parser.add_argument("-b", "--backend", choices=["openocd", "jlink"], help="RTT backend (openocd or jlink)")
    parser.add_argument("-f", "--file", help="Read from binary file instead of RTT")
    parser.add_argument("-c", "--channel", type=int, default=1, help="RTT channel number (default: 1)")
    parser.add_argument("-d", "--device", default="STM32F205RB", help="Target device (for J-Link, default: STM32F205RB)")
    parser.add_argument("-i", "--interface", default="SWD", choices=["SWD", "JTAG"], help="Debug interface (for J-Link, default: SWD)")
    parser.add_argument("--host", default="localhost", help="OpenOCD host (default: localhost)")
    parser.add_argument("-p", "--port", type=int, default=4444, help="OpenOCD port (default: 4444)")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose output")

    args = parser.parse_args()

    # Read from file or RTT
    if args.file:
        read_from_file(args.file, args.verbose)
    elif args.backend:
        read_from_rtt(backend=args.backend, channel=args.channel, device=args.device, interface=args.interface, host=args.host, port=args.port, verbose=args.verbose)
    else:
        parser.print_help()
        sys.exit(1)


if __name__ == "__main__":
    main()
