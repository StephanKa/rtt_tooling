"""Unit tests for rtt_analyzer.py."""

from pathlib import Path

from rtt_analyzer import LogEntry, RttLogAnalyzer


class TestLogEntry:
    """Test LogEntry class."""

    def test_log_entry_creation(self) -> None:
        """Test creating a log entry."""
        entry = LogEntry("INFO", "Test message")
        assert entry.level == "INFO"
        assert entry.message == "Test message"
        assert entry.timestamp is None

    def test_log_entry_with_timestamp(self) -> None:
        """Test creating a log entry with timestamp."""
        entry = LogEntry("DEBUG", "Debug message", "12:34:56")
        assert entry.level == "DEBUG"
        assert entry.message == "Debug message"
        assert entry.timestamp == "12:34:56"

    def test_log_entry_repr(self) -> None:
        """Test log entry string representation."""
        entry = LogEntry("ERROR", "Error occurred")
        assert str(entry) == "ERROR: Error occurred"

        entry_with_ts = LogEntry("WARN", "Warning", "10:20:30")
        assert str(entry_with_ts) == "[10:20:30] WARN: Warning"


class TestRttLogAnalyzer:
    """Test RttLogAnalyzer class."""

    def test_analyzer_creation(self, temp_dir: Path) -> None:
        """Test creating an analyzer."""
        log_file = temp_dir / "test.log"
        log_file.write_text("[INFO] Test\n")
        analyzer = RttLogAnalyzer(log_file)
        assert analyzer.log_file == log_file
        assert analyzer.entries == []

    def test_parse_valid_log(self, temp_dir: Path) -> None:
        """Test parsing a valid log file."""
        log_file = temp_dir / "test.log"
        log_file.write_text(
            """[INFO] Application started
[DEBUG] Debug message
[WARN] Warning message
[ERROR] Error message
"""
        )

        analyzer = RttLogAnalyzer(log_file)
        assert analyzer.parse()
        assert len(analyzer.entries) == 4
        assert analyzer.entries[0].level == "INFO"
        assert analyzer.entries[0].message == "Application started"
        assert analyzer.entries[1].level == "DEBUG"
        assert analyzer.entries[2].level == "WARN"
        assert analyzer.entries[3].level == "ERROR"

    def test_parse_multiline_message(self, temp_dir: Path) -> None:
        """Test parsing multiline log messages."""
        log_file = temp_dir / "test.log"
        log_file.write_text(
            """[ERROR] Exception occurred
Stack trace line 1
Stack trace line 2
[INFO] Recovery successful
"""
        )

        analyzer = RttLogAnalyzer(log_file)
        assert analyzer.parse()
        assert len(analyzer.entries) == 2
        assert "Stack trace line 1" in analyzer.entries[0].message
        assert "Stack trace line 2" in analyzer.entries[0].message

    def test_parse_empty_file(self, temp_dir: Path) -> None:
        """Test parsing an empty log file."""
        log_file = temp_dir / "empty.log"
        log_file.write_text("")

        analyzer = RttLogAnalyzer(log_file)
        assert analyzer.parse()
        assert len(analyzer.entries) == 0

    def test_parse_nonexistent_file(self, temp_dir: Path) -> None:
        """Test parsing a non-existent file."""
        log_file = temp_dir / "nonexistent.log"
        analyzer = RttLogAnalyzer(log_file)
        assert not analyzer.parse()

    def test_get_statistics(self, temp_dir: Path) -> None:
        """Test getting log statistics."""
        log_file = temp_dir / "test.log"
        log_file.write_text(
            """[INFO] Message 1
[INFO] Message 2
[DEBUG] Debug message
[ERROR] Error message
[ERROR] Another error
"""
        )

        analyzer = RttLogAnalyzer(log_file)
        analyzer.parse()
        stats = analyzer.get_statistics()

        assert stats["INFO"] == 2
        assert stats["DEBUG"] == 1
        assert stats["ERROR"] == 2

    def test_filter_by_level(self, temp_dir: Path) -> None:
        """Test filtering entries by log level."""
        log_file = temp_dir / "test.log"
        log_file.write_text(
            """[INFO] Info 1
[DEBUG] Debug 1
[INFO] Info 2
[ERROR] Error 1
"""
        )

        analyzer = RttLogAnalyzer(log_file)
        analyzer.parse()

        info_entries = analyzer.filter_by_level("INFO")
        assert len(info_entries) == 2
        assert all(e.level == "INFO" for e in info_entries)

        error_entries = analyzer.filter_by_level("ERROR")
        assert len(error_entries) == 1
        assert error_entries[0].level == "ERROR"

    def test_search_case_insensitive(self, temp_dir: Path) -> None:
        """Test case-insensitive search."""
        log_file = temp_dir / "test.log"
        log_file.write_text(
            """[INFO] Connection established
[DEBUG] Debugging connection
[ERROR] Connection failed
"""
        )

        analyzer = RttLogAnalyzer(log_file)
        analyzer.parse()

        results = analyzer.search("connection", case_sensitive=False)
        assert len(results) == 3

    def test_search_case_sensitive(self, temp_dir: Path) -> None:
        """Test case-sensitive search."""
        log_file = temp_dir / "test.log"
        log_file.write_text(
            """[INFO] Connection established
[DEBUG] Debugging connection
[ERROR] CONNECTION failed
"""
        )

        analyzer = RttLogAnalyzer(log_file)
        analyzer.parse()

        results = analyzer.search("Connection", case_sensitive=True)
        assert len(results) == 1
        assert "Connection established" in results[0].message

    def test_search_with_regex(self, temp_dir: Path) -> None:
        """Test search with regex pattern."""
        log_file = temp_dir / "test.log"
        log_file.write_text(
            """[INFO] Error code: 404
[DEBUG] Status: 200
[ERROR] Error code: 500
"""
        )

        analyzer = RttLogAnalyzer(log_file)
        analyzer.parse()

        results = analyzer.search(r"Error code: \d+")
        assert len(results) == 2
