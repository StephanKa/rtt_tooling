"""Unit tests for rtt_data_reader.py."""

import struct

from rtt_data_reader import DataHeader, DataType, RttDataReader


class TestDataType:
    """Test DataType enum."""

    def test_data_type_values(self) -> None:
        """Test data type enum values."""
        assert DataType.Int8 == 0
        assert DataType.UInt8 == 1
        assert DataType.String == 10
        assert DataType.Binary == 11


class TestDataHeader:
    """Test DataHeader dataclass."""

    def test_header_creation(self) -> None:
        """Test creating a data header."""
        header = DataHeader(magic=b"RD", data_type=DataType.Int32, reserved=0, size=4, timestamp=12345)
        assert header.magic == b"RD"
        assert header.data_type == DataType.Int32
        assert header.reserved == 0
        assert header.size == 4
        assert header.timestamp == 12345


class TestRttDataReader:
    """Test RttDataReader class."""

    def test_reader_creation(self) -> None:
        """Test creating a data reader."""
        reader = RttDataReader(verbose=False)
        assert reader.verbose is False
        assert reader.packet_count == 0
        assert reader.error_count == 0

    def test_parse_valid_header(self) -> None:
        """Test parsing a valid header."""
        reader = RttDataReader()
        # Create a valid header: magic (2 bytes), type (1 byte), reserved (1 byte), size (4 bytes), timestamp (4 bytes)
        header_data = struct.pack("<2sBBII", b"RD", DataType.Int32, 0, 4, 12345)

        header = reader.parse_header(header_data)
        assert header is not None
        assert header.magic == b"RD"
        assert header.data_type == DataType.Int32
        assert header.size == 4
        assert header.timestamp == 12345

    def test_parse_invalid_magic(self) -> None:
        """Test parsing with invalid magic bytes."""
        reader = RttDataReader()
        header_data = struct.pack("<2sBBII", b"XX", DataType.Int32, 0, 4, 12345)

        header = reader.parse_header(header_data)
        assert header is None

    def test_parse_invalid_data_type(self) -> None:
        """Test parsing with invalid data type."""
        reader = RttDataReader()
        header_data = struct.pack("<2sBBII", b"RD", 255, 0, 4, 12345)  # Invalid type

        header = reader.parse_header(header_data)
        assert header is None

    def test_parse_insufficient_data(self) -> None:
        """Test parsing with insufficient data."""
        reader = RttDataReader()
        header_data = b"RD"  # Too short

        header = reader.parse_header(header_data)
        assert header is None

    def test_parse_int32_data(self) -> None:
        """Test parsing Int32 data."""
        reader = RttDataReader()
        header = DataHeader(magic=b"RD", data_type=DataType.Int32, reserved=0, size=4, timestamp=0)
        data = struct.pack("<i", -42)

        value = reader.parse_data(header, data)
        assert value == -42

    def test_parse_uint32_data(self) -> None:
        """Test parsing UInt32 data."""
        reader = RttDataReader()
        header = DataHeader(magic=b"RD", data_type=DataType.UInt32, reserved=0, size=4, timestamp=0)
        data = struct.pack("<I", 42)

        value = reader.parse_data(header, data)
        assert value == 42

    def test_parse_float_data(self) -> None:
        """Test parsing Float data."""
        reader = RttDataReader()
        header = DataHeader(magic=b"RD", data_type=DataType.Float, reserved=0, size=4, timestamp=0)
        data = struct.pack("<f", 3.14)

        value = reader.parse_data(header, data)
        assert abs(value - 3.14) < 0.01  # Float comparison with tolerance

    def test_parse_string_data(self) -> None:
        """Test parsing String data."""
        reader = RttDataReader()
        test_string = "Hello, World!"
        header = DataHeader(magic=b"RD", data_type=DataType.String, reserved=0, size=len(test_string), timestamp=0)
        data = test_string.encode("utf-8")

        value = reader.parse_data(header, data)
        assert value == test_string

    def test_parse_binary_data(self) -> None:
        """Test parsing Binary data."""
        reader = RttDataReader()
        binary_data = b"\x01\x02\x03\x04"
        header = DataHeader(magic=b"RD", data_type=DataType.Binary, reserved=0, size=len(binary_data), timestamp=0)

        value = reader.parse_data(header, binary_data)
        assert value == "01020304"

    def test_format_value_string(self) -> None:
        """Test formatting a string value."""
        reader = RttDataReader()
        header = DataHeader(magic=b"RD", data_type=DataType.String, reserved=0, size=5, timestamp=0)
        formatted = reader.format_value(header, "Hello")
        assert '[String] "Hello"' in formatted

    def test_format_value_binary(self) -> None:
        """Test formatting a binary value."""
        reader = RttDataReader()
        header = DataHeader(magic=b"RD", data_type=DataType.Binary, reserved=0, size=4, timestamp=0)
        formatted = reader.format_value(header, "01020304")
        assert "[Binary]" in formatted
        assert "0x01020304" in formatted

    def test_format_value_float(self) -> None:
        """Test formatting a float value."""
        reader = RttDataReader()
        header = DataHeader(magic=b"RD", data_type=DataType.Float, reserved=0, size=4, timestamp=0)
        formatted = reader.format_value(header, 3.14159)
        assert "[Float]" in formatted
        assert "3.14159" in formatted

    def test_process_complete_packet(self) -> None:
        """Test processing a complete packet."""
        reader = RttDataReader()

        # Create a complete packet
        header_data = struct.pack("<2sBBII", b"RD", DataType.Int32, 0, 4, 12345)
        payload_data = struct.pack("<i", 42)
        packet_data = header_data + payload_data

        _value, consumed = reader.process_packet(packet_data)
        assert consumed == RttDataReader.HEADER_SIZE + 4
        assert reader.packet_count == 1
        assert reader.error_count == 0

    def test_process_incomplete_packet(self) -> None:
        """Test processing an incomplete packet."""
        reader = RttDataReader()

        # Create incomplete packet (header only)
        header_data = struct.pack("<2sBBII", b"RD", DataType.Int32, 0, 4, 12345)

        _value, consumed = reader.process_packet(header_data)
        assert consumed == 0  # Not enough data
        assert reader.packet_count == 0
