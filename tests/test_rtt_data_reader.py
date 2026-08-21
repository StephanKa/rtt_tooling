"""Unit tests for rtt_data_reader.py."""

import struct
import zlib

from hypothesis import given, strategies as st
from rtt_data_reader import DataHeader, DataType, RttDataReader


def make_header(data_type: DataType, payload: bytes, timestamp: int = 0, sequence: int = 0) -> bytes:
    """Build a versioned data packet header for tests."""
    return struct.pack(
        RttDataReader.HEADER_FORMAT,
        RttDataReader.MAGIC_BYTES,
        RttDataReader.PROTOCOL_VERSION,
        data_type,
        len(payload),
        timestamp,
        sequence,
        zlib.crc32(payload),
    )


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
        header = DataHeader(magic=b"RD", version=2, data_type=DataType.Int32, size=4, timestamp=12345, sequence=7, payload_crc32=0)
        assert header.magic == b"RD"
        assert header.version == 2
        assert header.data_type == DataType.Int32
        assert header.size == 4
        assert header.timestamp == 12345
        assert header.sequence == 7


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
        payload = struct.pack("<i", 42)
        header_data = make_header(DataType.Int32, payload, timestamp=12345, sequence=9)

        header = reader.parse_header(header_data)
        assert header is not None
        assert header.magic == b"RD"
        assert header.data_type == DataType.Int32
        assert header.size == 4
        assert header.timestamp == 12345
        assert header.sequence == 9

    def test_parse_invalid_magic(self) -> None:
        """Test parsing with invalid magic bytes."""
        reader = RttDataReader()
        header_data = struct.pack(RttDataReader.HEADER_FORMAT, b"XX", 2, DataType.Int32, 4, 12345, 0, 0)

        header = reader.parse_header(header_data)
        assert header is None

    def test_parse_invalid_data_type(self) -> None:
        """Test parsing with invalid data type."""
        reader = RttDataReader()
        header_data = struct.pack(RttDataReader.HEADER_FORMAT, b"RD", 2, 255, 4, 12345, 0, 0)

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
        header = DataHeader(b"RD", 2, DataType.Int32, 4, 0, 0, 0)
        data = struct.pack("<i", -42)

        value = reader.parse_data(header, data)
        assert value == -42

    def test_parse_uint32_data(self) -> None:
        """Test parsing UInt32 data."""
        reader = RttDataReader()
        header = DataHeader(b"RD", 2, DataType.UInt32, 4, 0, 0, 0)
        data = struct.pack("<I", 42)

        value = reader.parse_data(header, data)
        assert value == 42

    def test_parse_float_data(self) -> None:
        """Test parsing Float data."""
        reader = RttDataReader()
        header = DataHeader(b"RD", 2, DataType.Float, 4, 0, 0, 0)
        data = struct.pack("<f", 3.14)

        value = reader.parse_data(header, data)
        assert abs(value - 3.14) < 0.01  # Float comparison with tolerance

    def test_parse_string_data(self) -> None:
        """Test parsing String data."""
        reader = RttDataReader()
        test_string = "Hello, World!"
        header = DataHeader(b"RD", 2, DataType.String, len(test_string), 0, 0, 0)
        data = test_string.encode("utf-8")

        value = reader.parse_data(header, data)
        assert value == test_string

    def test_parse_binary_data(self) -> None:
        """Test parsing Binary data."""
        reader = RttDataReader()
        binary_data = b"\x01\x02\x03\x04"
        header = DataHeader(b"RD", 2, DataType.Binary, len(binary_data), 0, 0, 0)

        value = reader.parse_data(header, binary_data)
        assert value == "01020304"

    def test_format_value_string(self) -> None:
        """Test formatting a string value."""
        reader = RttDataReader()
        header = DataHeader(b"RD", 2, DataType.String, 5, 0, 0, 0)
        formatted = reader.format_value(header, "Hello")
        assert '[String] "Hello"' in formatted

    def test_format_value_binary(self) -> None:
        """Test formatting a binary value."""
        reader = RttDataReader()
        header = DataHeader(b"RD", 2, DataType.Binary, 4, 0, 0, 0)
        formatted = reader.format_value(header, "01020304")
        assert "[Binary]" in formatted
        assert "0x01020304" in formatted

    def test_format_value_float(self) -> None:
        """Test formatting a float value."""
        reader = RttDataReader()
        header = DataHeader(b"RD", 2, DataType.Float, 4, 0, 0, 0)
        formatted = reader.format_value(header, 3.14159)
        assert "[Float]" in formatted
        assert "3.14159" in formatted

    def test_process_complete_packet(self) -> None:
        """Test processing a complete packet."""
        reader = RttDataReader()

        # Create a complete packet
        payload_data = struct.pack("<i", 42)
        header_data = make_header(DataType.Int32, payload_data, timestamp=12345)
        packet_data = header_data + payload_data

        _value, consumed = reader.process_packet(packet_data)
        assert consumed == RttDataReader.HEADER_SIZE + 4
        assert reader.packet_count == 1
        assert reader.error_count == 0

    def test_process_incomplete_packet(self) -> None:
        """Test processing an incomplete packet."""
        reader = RttDataReader()

        # Create incomplete packet (header only)
        payload_data = struct.pack("<i", 42)
        header_data = make_header(DataType.Int32, payload_data, timestamp=12345)

        _value, consumed = reader.process_packet(header_data)
        assert consumed == 0  # Not enough data
        assert reader.packet_count == 0

    def test_rejects_oversized_payload_without_stalling(self) -> None:
        """An impossible payload length advances the stream parser."""
        reader = RttDataReader()
        header_data = struct.pack(RttDataReader.HEADER_FORMAT, b"RD", 2, DataType.Binary, 0xFFFFFFFF, 0, 0, 0)

        _value, consumed = reader.process_packet(header_data)

        assert consumed > 0
        assert reader.error_count == 1

    def test_recovers_at_next_packet_magic(self) -> None:
        """Corrupt bytes before a packet are consumed up to the next magic marker."""
        reader = RttDataReader()
        payload = struct.pack("<I", 42)
        packet = make_header(DataType.UInt32, payload) + payload

        _value, consumed = reader.process_packet(b"noise" + packet)

        assert consumed == len(b"noise")

    def test_rejects_bad_payload_crc(self) -> None:
        """A complete packet with corrupt payload is discarded."""
        reader = RttDataReader()
        payload = struct.pack("<I", 42)
        packet = make_header(DataType.UInt32, payload) + b"bad!"

        value, consumed = reader.process_packet(packet)

        assert value is None
        assert consumed == len(packet)
        assert reader.error_count == 1

    @given(st.binary(max_size=1024))
    def test_arbitrary_input_never_overconsumes(self, data: bytes) -> None:
        """Malformed streams are rejected without exceptions or invalid consumption."""
        reader = RttDataReader()

        _value, consumed = reader.process_packet(data)

        assert 0 <= consumed <= len(data)
