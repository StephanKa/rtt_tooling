#!/usr/bin/env python3
"""
RTT Logger Analyzer

This script analyzes RTT log files and provides statistics and filtering capabilities.
"""

import argparse
import re
import sys
from collections import Counter
from pathlib import Path
from typing import Dict, List, Optional


class LogEntry:
    """Represents a single log entry"""

    def __init__(self, level: str, message: str, timestamp: Optional[str] = None):
        self.level = level
        self.message = message
        self.timestamp = timestamp

    def __repr__(self):
        if self.timestamp:
            return f"[{self.timestamp}] {self.level}: {self.message}"
        return f"{self.level}: {self.message}"


class RttLogAnalyzer:
    """Analyzer for RTT log files"""

    LOG_PATTERN = re.compile(r"\[(TRACE|DEBUG|INFO|WARN|ERROR|CRIT)\]\s+(.*)")

    def __init__(self, log_file: Path):
        self.log_file = log_file
        self.entries: List[LogEntry] = []

    def parse(self) -> bool:
        """
        Parse log file

        Returns:
            True if parsing successful, False otherwise
        """
        try:
            with open(self.log_file, encoding="utf-8") as f:
                for line in f:
                    line = line.strip()
                    if not line:
                        continue

                    match = self.LOG_PATTERN.match(line)
                    if match:
                        level = match.group(1)
                        message = match.group(2)
                        self.entries.append(LogEntry(level, message))
                    # Line doesn't match pattern, might be continuation
                    elif self.entries:
                        self.entries[-1].message += "\n" + line

            return True
        except Exception as e:
            print(f"Error parsing log file: {e}", file=sys.stderr)
            return False

    def get_statistics(self) -> Dict[str, int]:
        """
        Get log level statistics

        Returns:
            Dictionary mapping log levels to counts
        """
        counter = Counter(entry.level for entry in self.entries)
        return dict(counter)

    def filter_by_level(self, level: str) -> List[LogEntry]:
        """
        Filter entries by log level

        Args:
            level: Log level to filter by

        Returns:
            List of matching entries
        """
        return [entry for entry in self.entries if entry.level == level]

    def search(self, pattern: str, case_sensitive: bool = False) -> List[LogEntry]:
        """
        Search for pattern in log messages

        Args:
            pattern: Search pattern (regex)
            case_sensitive: Whether search is case-sensitive

        Returns:
            List of matching entries
        """
        flags = 0 if case_sensitive else re.IGNORECASE
        regex = re.compile(pattern, flags)
        return [entry for entry in self.entries if regex.search(entry.message)]

    def print_summary(self):
        """Print summary statistics"""
        stats = self.get_statistics()
        total = len(self.entries)

        print("\n=== Log Analysis Summary ===")
        print(f"Total entries: {total}")
        print("\nBreakdown by level:")
        for level in ["TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRIT"]:
            count = stats.get(level, 0)
            percentage = (count / total * 100) if total > 0 else 0
            print(f"  {level:8s}: {count:6d} ({percentage:5.1f}%)")


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description="RTT Log Analyzer - Analyze and filter RTT log files")
    parser.add_argument("logfile", type=Path, help="Log file to analyze")
    parser.add_argument("-l", "--level", choices=["TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRIT"], help="Filter by log level")
    parser.add_argument("-s", "--search", help="Search pattern (regex)")
    parser.add_argument("-i", "--ignore-case", action="store_true", help="Case-insensitive search")
    parser.add_argument("--stats-only", action="store_true", help="Show only statistics")

    args = parser.parse_args()

    if not args.logfile.exists():
        print(f"Error: Log file '{args.logfile}' not found", file=sys.stderr)
        return 1

    analyzer = RttLogAnalyzer(args.logfile)

    if not analyzer.parse():
        return 1

    # Show statistics
    analyzer.print_summary()

    if args.stats_only:
        return 0

    # Apply filters
    entries = analyzer.entries

    if args.level:
        entries = analyzer.filter_by_level(args.level)
        print(f"\nFiltered by level '{args.level}': {len(entries)} entries")

    if args.search:
        entries = [e for e in entries if re.search(args.search, e.message, re.IGNORECASE if args.ignore_case else 0)]
        print(f"Search results for '{args.search}': {len(entries)} entries")

    # Print entries
    if entries and not args.stats_only:
        print("\n=== Log Entries ===")
        for entry in entries:
            print(entry)

    return 0


if __name__ == "__main__":
    sys.exit(main())
