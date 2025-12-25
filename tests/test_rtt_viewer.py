"""Unit tests for rtt_viewer.py."""

from rtt_viewer import RttViewer


class TestRttViewer:
    """Test RttViewer class."""

    def test_viewer_creation(self) -> None:
        """Test creating an RTT viewer."""
        viewer = RttViewer(device="STM32F205RB", interface="SWD", speed=4000)
        assert viewer.device == "STM32F205RB"
        assert viewer.interface == "SWD"
        assert viewer.speed == 4000
        assert not viewer.running

    def test_viewer_with_custom_device(self) -> None:
        """Test creating viewer with custom device."""
        viewer = RttViewer(device="STM32F407", interface="JTAG", speed=8000)
        assert viewer.device == "STM32F407"
        assert viewer.interface == "JTAG"
        assert viewer.speed == 8000

    def test_viewer_default_parameters(self) -> None:
        """Test creating viewer with default parameters."""
        viewer = RttViewer()
        assert viewer.device == "STM32F205RB"
        assert viewer.interface == "SWD"
        assert viewer.speed == 4000

    def test_disconnect(self) -> None:
        """Test disconnecting viewer."""
        viewer = RttViewer()
        viewer.running = True
        viewer.disconnect()
        assert not viewer.running

    def test_read_rtt_placeholder(self) -> None:
        """Test read_rtt placeholder implementation."""
        viewer = RttViewer()
        data = viewer.read_rtt(channel=0)
        assert data is None  # Placeholder returns None
