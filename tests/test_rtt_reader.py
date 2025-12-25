"""Unit tests for rtt_reader.py."""

from rtt_reader import JLinkRttReader, OpenOcdRttReader, RttBackend, RttReaderApp


class TestRttBackend:
    """Test RttBackend enum."""

    def test_backend_values(self) -> None:
        """Test backend enum values."""
        assert RttBackend.OPENOCD.value == "openocd"
        assert RttBackend.JLINK.value == "jlink"


class TestOpenOcdRttReader:
    """Test OpenOcdRttReader class."""

    def test_reader_creation(self) -> None:
        """Test creating an OpenOCD RTT reader."""
        reader = OpenOcdRttReader(host="localhost", port=4444)
        assert reader.host == "localhost"
        assert reader.port == 4444
        assert reader.socket is None
        assert not reader._connected

    def test_reader_with_custom_host(self) -> None:
        """Test creating reader with custom host."""
        reader = OpenOcdRttReader(host="192.168.1.100", port=5555)
        assert reader.host == "192.168.1.100"
        assert reader.port == 5555

    def test_is_connected_initially_false(self) -> None:
        """Test is_connected returns False initially."""
        reader = OpenOcdRttReader()
        assert not reader.is_connected()

    def test_disconnect_without_connection(self) -> None:
        """Test disconnecting without connection doesn't raise error."""
        reader = OpenOcdRttReader()
        reader.disconnect()  # Should not raise
        assert not reader.is_connected()

    def test_read_rtt_when_not_connected(self) -> None:
        """Test reading RTT when not connected returns None."""
        reader = OpenOcdRttReader()
        data = reader.read_rtt()
        assert data is None


class TestJLinkRttReader:
    """Test JLinkRttReader class - mock tests since pylink may not be available."""

    def test_reader_creation(self) -> None:
        """Test creating a J-Link RTT reader."""
        reader = JLinkRttReader(device="STM32F205RB", interface="SWD", speed=4000)
        assert reader.device == "STM32F205RB"
        assert reader.interface == "SWD"
        assert reader.speed == 4000
        assert reader.jlink is None
        assert not reader._connected

    def test_reader_with_custom_device(self) -> None:
        """Test creating reader with custom device."""
        reader = JLinkRttReader(device="STM32F407", interface="JTAG", speed=8000)
        assert reader.device == "STM32F407"
        assert reader.interface == "JTAG"
        assert reader.speed == 8000

    def test_is_connected_initially_false(self) -> None:
        """Test is_connected returns False initially."""
        reader = JLinkRttReader()
        assert not reader.is_connected()

    def test_disconnect_without_connection(self) -> None:
        """Test disconnecting without connection doesn't raise error."""
        reader = JLinkRttReader()
        reader.disconnect()  # Should not raise
        assert not reader.is_connected()

    def test_read_rtt_when_not_connected(self) -> None:
        """Test reading RTT when not connected returns None."""
        reader = JLinkRttReader()
        data = reader.read_rtt()
        assert data is None


class TestRttReaderApp:
    """Test RttReaderApp class."""

    def test_app_creation(self) -> None:
        """Test creating an RTT reader application."""
        reader = OpenOcdRttReader()
        app = RttReaderApp(reader, output_file=None)
        assert app.reader == reader
        assert app.output_file is None
        assert not app.running

    def test_app_with_output_file(self) -> None:
        """Test creating app with output file."""
        reader = OpenOcdRttReader()
        app = RttReaderApp(reader, output_file="output.txt")
        assert app.output_file == "output.txt"
