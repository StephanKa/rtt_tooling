"""Unit tests for rtt_trace_analyzer.py."""

import json
import struct
from pathlib import Path

from rtt_trace_analyzer import TraceAnalyzer, TraceEvent, TraceParser


class TestTraceEvent:
    """Test TraceEvent dataclass."""

    def test_trace_event_creation(self) -> None:
        """Test creating a trace event."""
        event = TraceEvent(event_type=0x01, event_name="TASK_SWITCHED_IN", timestamp=12345, handle=0x20000100, data=0)
        assert event.event_type == 0x01
        assert event.event_name == "TASK_SWITCHED_IN"
        assert event.timestamp == 12345
        assert event.handle == 0x20000100
        assert event.data == 0


class TestTraceParser:
    """Test TraceParser class."""

    def test_parser_creation(self, temp_dir: Path) -> None:
        """Test creating a trace parser."""
        trace_file = temp_dir / "trace.bin"
        trace_file.write_bytes(b"\x00" * 100)
        parser = TraceParser(trace_file)
        assert parser.trace_file == trace_file
        assert parser.events == []
        assert parser.task_registry == {}

    def test_parse_empty_file(self, temp_dir: Path) -> None:
        """Test parsing an empty trace file."""
        trace_file = temp_dir / "empty.bin"
        trace_file.write_bytes(b"")
        parser = TraceParser(trace_file)
        assert parser.parse()
        assert len(parser.events) == 0

    def test_parse_valid_event(self, temp_dir: Path) -> None:
        """Test parsing a valid trace event."""
        trace_file = temp_dir / "trace.bin"
        # Create a valid event: event_type (1 byte), timestamp (4 bytes), handle (4 bytes), data (4 bytes)
        event_data = struct.pack("<BIII", 0x01, 12345, 0x20000100, 0)
        trace_file.write_bytes(event_data)

        parser = TraceParser(trace_file)
        assert parser.parse()
        assert len(parser.events) == 1
        assert parser.events[0].event_type == 0x01
        assert parser.events[0].event_name == "TASK_SWITCHED_IN"
        assert parser.events[0].timestamp == 12345

    def test_parse_multiple_events(self, temp_dir: Path) -> None:
        """Test parsing multiple trace events."""
        trace_file = temp_dir / "trace.bin"
        events_data = b""
        # Create multiple events
        for event_type in [0x01, 0x02, 0x10, 0x11]:
            events_data += struct.pack("<BIII", event_type, 12345, 0x20000100, 0)
        trace_file.write_bytes(events_data)

        parser = TraceParser(trace_file)
        assert parser.parse()
        assert len(parser.events) == 4

    def test_parse_with_task_registry(self, temp_dir: Path) -> None:
        """Test parsing trace with task registry."""
        trace_file = temp_dir / "trace.bin"
        # Create task registry text
        registry_text = "TASK_REGISTRY_START\nTASK:537133312:IdleTask\nTASK:537133568:MainTask\nTASK_REGISTRY_END\n"
        trace_file.write_text(registry_text)

        parser = TraceParser(trace_file)
        parser.parse()
        assert len(parser.task_registry) == 2
        assert parser.task_registry[537133312] == "IdleTask"
        assert parser.task_registry[537133568] == "MainTask"

    def test_get_task_name_registered(self, temp_dir: Path) -> None:
        """Test getting task name for registered task."""
        trace_file = temp_dir / "trace.bin"
        trace_file.write_bytes(b"")
        parser = TraceParser(trace_file)
        parser.task_registry[0x20000100] = "TestTask"

        name = parser.get_task_name(0x20000100)
        assert name == "TestTask"

    def test_get_task_name_unregistered(self, temp_dir: Path) -> None:
        """Test getting task name for unregistered task."""
        trace_file = temp_dir / "trace.bin"
        trace_file.write_bytes(b"")
        parser = TraceParser(trace_file)

        name = parser.get_task_name(0x20000100)
        assert "Task_0x20000100" in name

    def test_timestamp_to_seconds(self, temp_dir: Path) -> None:
        """Test converting timestamp to seconds."""
        trace_file = temp_dir / "trace.bin"
        trace_file.write_bytes(b"")
        parser = TraceParser(trace_file)
        parser.cpu_frequency = 168000000  # 168 MHz

        seconds = parser.timestamp_to_seconds(168000000)
        assert abs(seconds - 1.0) < 0.001  # Should be 1 second


class TestTraceAnalyzer:
    """Test TraceAnalyzer class."""

    def test_analyzer_creation(self, temp_dir: Path) -> None:
        """Test creating a trace analyzer."""
        trace_file = temp_dir / "trace.bin"
        trace_file.write_bytes(b"")
        parser = TraceParser(trace_file)
        parser.parse()

        analyzer = TraceAnalyzer(parser)
        assert analyzer.parser == parser
        assert analyzer.events == parser.events

    def test_analyze_empty_trace(self, temp_dir: Path) -> None:
        """Test analyzing empty trace."""
        trace_file = temp_dir / "trace.bin"
        trace_file.write_bytes(b"")
        parser = TraceParser(trace_file)
        parser.parse()

        analyzer = TraceAnalyzer(parser)
        # Should not raise any errors
        analyzer.print_summary()

    def test_export_json(self, temp_dir: Path) -> None:
        """Test exporting trace to JSON."""
        trace_file = temp_dir / "trace.bin"
        # Create a simple event
        event_data = struct.pack("<BIII", 0x01, 12345, 0x20000100, 0)
        trace_file.write_bytes(event_data)

        parser = TraceParser(trace_file)
        parser.parse()

        analyzer = TraceAnalyzer(parser)
        output_file = temp_dir / "trace.json"
        analyzer.export_json(output_file)

        assert output_file.exists()
        with output_file.open() as f:
            data = json.load(f)
            assert "metadata" in data
            assert "events" in data
            assert len(data["events"]) == 1

    def test_export_chrome_trace(self, temp_dir: Path) -> None:
        """Test exporting trace to Chrome Trace format."""
        trace_file = temp_dir / "trace.bin"
        # Create a simple event
        event_data = struct.pack("<BIII", 0x01, 12345, 0x20000100, 0)
        trace_file.write_bytes(event_data)

        parser = TraceParser(trace_file)
        parser.parse()

        analyzer = TraceAnalyzer(parser)
        output_file = temp_dir / "trace_chrome.json"
        analyzer.export_chrome_trace(output_file)

        assert output_file.exists()
        with output_file.open() as f:
            data = json.load(f)
            assert "traceEvents" in data
            assert "metadata" in data
