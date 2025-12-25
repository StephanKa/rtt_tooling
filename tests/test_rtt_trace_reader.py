"""Unit tests for rtt_trace_reader.py."""

from pathlib import Path

import pytest
from rtt_trace_reader import TRACE_EVENTS, JLinkRttReader, OpenOcdRttReader, TraceReader


class TestTraceEvents:
    """Test TRACE_EVENTS dictionary."""

    def test_trace_events_defined(self) -> None:
        """Test that trace events are defined."""
        assert 0x01 in TRACE_EVENTS
        assert TRACE_EVENTS[0x01] == "TASK_SWITCHED_IN"
        assert TRACE_EVENTS[0x02] == "TASK_SWITCHED_OUT"
        assert TRACE_EVENTS[0x10] == "ISR_ENTER"
        assert TRACE_EVENTS[0x11] == "ISR_EXIT"


class TestJLinkRttReader:
    """Test JLinkRttReader class."""

    def test_reader_creation(self) -> None:
        """Test creating a J-Link RTT reader."""
        reader = JLinkRttReader(device="STM32F205RB", channel=1)
        assert reader.device == "STM32F205RB"
        assert reader.channel == 1
        assert reader.jlink is None

    def test_reader_with_custom_device(self) -> None:
        """Test creating reader with custom device."""
        reader = JLinkRttReader(device="STM32F407", channel=2)
        assert reader.device == "STM32F407"
        assert reader.channel == 2


class TestOpenOcdRttReader:
    """Test OpenOcdRttReader class."""

    def test_reader_creation(self) -> None:
        """Test creating an OpenOCD RTT reader."""
        reader = OpenOcdRttReader(device="stm32f2x", channel=1)
        assert reader.device == "stm32f2x"
        assert reader.channel == 1
        assert reader.telnet is None

    def test_reader_with_custom_host(self) -> None:
        """Test creating reader with custom host and port."""
        reader = OpenOcdRttReader(device="stm32f2x", channel=1, host="192.168.1.100", port=5555)
        assert reader.host == "192.168.1.100"
        assert reader.port == 5555

    def test_disconnect_without_connection(self) -> None:
        """Test disconnecting without connection doesn't raise error."""
        reader = OpenOcdRttReader(device="stm32f2x", channel=1)
        reader.disconnect()  # Should not raise


class TestTraceReader:
    """Test TraceReader class."""

    def test_reader_creation_jlink(self) -> None:
        """Test creating a trace reader with J-Link."""
        reader = TraceReader(probe="jlink", device="STM32F205RB", channel=1, output_file=None)
        assert reader.probe == "jlink"
        assert reader.channel == 1
        assert reader.output_file is None
        assert not reader.running

    def test_reader_creation_openocd(self) -> None:
        """Test creating a trace reader with OpenOCD."""
        reader = TraceReader(probe="openocd", device="stm32f2x", channel=1, output_file=None)
        assert reader.probe == "openocd"

    def test_reader_creation_with_output_file(self, temp_dir: Path) -> None:
        """Test creating reader with output file."""
        output_file = temp_dir / "trace.bin"
        reader = TraceReader(probe="jlink", device="STM32F205RB", channel=1, output_file=output_file)
        assert reader.output_file == output_file

    def test_reader_creation_invalid_probe(self) -> None:
        """Test creating reader with invalid probe type."""
        with pytest.raises(ValueError, match="Unknown probe type"):
            TraceReader(probe="invalid", device="STM32F205RB", channel=1, output_file=None)
