#!/usr/bin/env python3
"""
FreeRTOS Trace Analyzer

This script parses and analyzes FreeRTOS trace data captured via RTT.
It provides statistics, timeline visualization, and performance analysis.

Usage:
    python3 rtt_trace_analyzer.py trace.bin
    python3 rtt_trace_analyzer.py trace.bin --timeline
    python3 rtt_trace_analyzer.py trace.bin --stats
    python3 rtt_trace_analyzer.py trace.bin --task-runtime
"""

import argparse
import json
import struct
import sys
from collections import Counter, defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List

# Chrome Trace format constants
CHROME_TRACE_ISR_THREAD_ID = 999999  # Special thread ID for ISRs in Chrome Trace

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


@dataclass
class TraceEvent:
    """Trace event structure"""

    event_type: int
    event_name: str
    timestamp: int
    handle: int
    data: int


class TraceParser:
    """Parse binary trace data"""

    # Struct format: B = uint8, I = uint32 (little-endian)
    EVENT_FORMAT = "<BIII"
    EVENT_SIZE = struct.calcsize(EVENT_FORMAT)

    def __init__(self, trace_file: Path):
        self.trace_file = trace_file
        self.events: List[TraceEvent] = []
        self.task_registry: Dict[int, str] = {}
        self.cpu_frequency = 168000000  # STM32F205 default (168 MHz)

    def parse(self) -> bool:
        """Parse trace file"""
        try:
            with open(self.trace_file, "rb") as f:
                content = f.read()

            print("Parsing trace file...")

            # Look for text markers and task registry
            self._parse_text_data(content)

            # Parse binary trace events
            self._parse_binary_events(content)

            print(f"Parsed {len(self.events)} trace events")
            print(f"Registered {len(self.task_registry)} tasks")
            if self.task_registry:
                print("Task registry:")
                for handle, name in self.task_registry.items():
                    print(f"  0x{handle:08X}: {name}")
            return True

        except Exception as e:
            print(f"Error parsing trace file: {e}", file=sys.stderr)
            return False

    def _parse_text_data(self, content: bytes):
        """Parse text markers and task registry"""
        try:
            # Try to decode as text to find markers
            text = content.decode("utf-8", errors="ignore")

            # Look for task registry
            if "TASK_REGISTRY_START" in text and "TASK_REGISTRY_END" in text:
                start_idx = text.index("TASK_REGISTRY_START")
                end_idx = text.index("TASK_REGISTRY_END")
                registry_text = text[start_idx:end_idx]

                # Parse task entries: TASK:handle:name
                for line in registry_text.split("\n"):
                    if line.startswith("TASK:"):
                        parts = line.split(":", 2)  # Split into max 3 parts (TASK, handle, name)
                        if len(parts) >= 3:
                            try:
                                handle = int(parts[1])
                                name = parts[2].strip()
                                self.task_registry[handle] = name
                                print(f"  Registered task: handle=0x{handle:08X} ({handle}), name='{name}'")
                            except ValueError as e:
                                print(f"  Warning: Failed to parse task entry '{line}': {e}")
            else:
                print("  Warning: No task registry found in trace data")
        except Exception as e:
            print(f"  Warning: Error parsing text data: {e}")

    def _parse_binary_events(self, content: bytes):
        """Parse binary trace events"""
        offset = 0

        while offset + self.EVENT_SIZE <= len(content):
            try:
                # Try to extract an event
                event_bytes = content[offset : offset + self.EVENT_SIZE]

                # Unpack event
                event_type, timestamp, handle, data = struct.unpack(self.EVENT_FORMAT, event_bytes)

                # Validate event type
                if event_type in TRACE_EVENTS:
                    event = TraceEvent(event_type=event_type, event_name=TRACE_EVENTS[event_type], timestamp=timestamp, handle=handle, data=data)
                    self.events.append(event)
                    offset += self.EVENT_SIZE
                else:
                    # Skip this byte and try next position
                    offset += 1
            except struct.error:
                # Not enough bytes for a full event
                break

    def get_task_name(self, handle: int) -> str:
        """Get task name from handle"""
        return self.task_registry.get(handle, f"Task_0x{handle:08X}")

    def timestamp_to_seconds(self, timestamp: int) -> float:
        """Convert cycle count to seconds"""
        return timestamp / self.cpu_frequency


class TraceAnalyzer:
    """Analyze parsed trace data"""

    def __init__(self, parser: TraceParser):
        self.parser = parser
        self.events = parser.events

    def print_summary(self):
        """Print summary statistics"""
        print("\n=== Trace Summary ===")
        print(f"Total events: {len(self.events)}")

        if not self.events:
            return

        # Event type breakdown
        event_counts = Counter(e.event_name for e in self.events)
        print("\nEvent type breakdown:")
        for event_name, count in sorted(event_counts.items()):
            percentage = (count / len(self.events)) * 100
            print(f"  {event_name:25s}: {count:6d} ({percentage:5.1f}%)")

        # Time range
        start_time = self.events[0].timestamp
        end_time = self.events[-1].timestamp
        duration = self.parser.timestamp_to_seconds(end_time - start_time)
        print(f"\nTrace duration: {duration:.6f} seconds")
        print(f"Start timestamp: {start_time}")
        print(f"End timestamp: {end_time}")

    def analyze_task_runtime(self):
        """Analyze task runtime statistics with detailed execution metrics"""
        print("\n=== Task Runtime Analysis ===")

        task_runtimes = defaultdict(int)  # Total execution time per task
        task_execution_count = defaultdict(int)  # Number of execution periods
        task_min_duration = defaultdict(lambda: float('inf'))  # Shortest execution period
        task_max_duration = defaultdict(int)  # Longest execution period
        current_task = None
        task_start_time = None

        # Validation counters
        unmatched_in = 0
        unmatched_out = 0

        for event in self.events:
            if event.event_name == "TASK_SWITCHED_IN":
                # Validation: Check if previous task switch-in wasn't closed
                if current_task is not None:
                    print(f"  Warning: Task 0x{current_task:08X} switched in without switching out", file=sys.stderr)
                    unmatched_in += 1

                current_task = event.handle
                task_start_time = event.timestamp

            elif event.event_name == "TASK_SWITCHED_OUT":
                # Validation: Check if this matches the current active task
                if current_task is None:
                    print(f"  Warning: Task 0x{event.handle:08X} switched out without switching in", file=sys.stderr)
                    unmatched_out += 1
                elif event.handle != current_task:
                    print(f"  Warning: Handle mismatch - active task 0x{current_task:08X} but switched out 0x{event.handle:08X}", file=sys.stderr)
                    # Still process the event for the handle that switched out
                    current_task = event.handle

                if current_task is not None and task_start_time is not None:
                    runtime = event.timestamp - task_start_time
                    if runtime < 0:
                        print(f"  Warning: Negative runtime for task 0x{current_task:08X} ({runtime} cycles)", file=sys.stderr)
                    else:
                        task_runtimes[current_task] += runtime
                        task_execution_count[current_task] += 1
                        task_min_duration[current_task] = min(task_min_duration[current_task], runtime)
                        task_max_duration[current_task] = max(task_max_duration[current_task], runtime)

                current_task = None
                task_start_time = None

        # Handle case where trace ends with a task still running
        if current_task is not None and task_start_time is not None:
            # Use last event timestamp as end time
            last_timestamp = self.events[-1].timestamp
            runtime = last_timestamp - task_start_time
            task_runtimes[current_task] += runtime
            task_execution_count[current_task] += 1
            task_min_duration[current_task] = min(task_min_duration[current_task], runtime)
            task_max_duration[current_task] = max(task_max_duration[current_task], runtime)
            print(f"  Note: Task 0x{current_task:08X} was still running when trace ended", file=sys.stderr)

        if not task_runtimes:
            print("No task switch events found")
            return

        # Show validation summary
        if unmatched_in > 0 or unmatched_out > 0:
            print(f"\nValidation warnings: {unmatched_in} unmatched SWITCHED_IN, {unmatched_out} unmatched SWITCHED_OUT events")

        # Calculate total runtime and trace duration
        total_runtime = sum(task_runtimes.values())
        trace_duration = self.events[-1].timestamp - self.events[0].timestamp

        # Calculate CPU utilization
        cpu_utilization = (total_runtime / trace_duration * 100) if trace_duration > 0 else 0

        print(f"\nTrace duration: {self.parser.timestamp_to_seconds(trace_duration):.6f} seconds")
        print(f"Total task runtime: {self.parser.timestamp_to_seconds(total_runtime):.6f} seconds")
        print(f"CPU utilization: {cpu_utilization:.1f}%")
        print(f"Idle time: {100 - cpu_utilization:.1f}%")
        print("\nTask runtime breakdown:")
        print(f"{'Task Name':<20} {'Runtime':>12} {'CPU %':>7} {'Executions':>12} {'Avg Time':>12} {'Min Time':>12} {'Max Time':>12}")
        print("-" * 110)

        # Sort by runtime (descending)
        sorted_tasks = sorted(task_runtimes.items(), key=lambda x: x[1], reverse=True)

        for handle, runtime in sorted_tasks:
            task_name = self.parser.get_task_name(handle)
            runtime_sec = self.parser.timestamp_to_seconds(runtime)
            cpu_percent = (runtime / trace_duration) * 100 if trace_duration > 0 else 0
            exec_count = task_execution_count[handle]
            avg_time = self.parser.timestamp_to_seconds(runtime / exec_count) if exec_count > 0 else 0
            min_time = self.parser.timestamp_to_seconds(task_min_duration[handle]) if task_min_duration[handle] != float('inf') else 0
            max_time = self.parser.timestamp_to_seconds(task_max_duration[handle])

            print(f"{task_name:<20} {runtime_sec:10.6f}s {cpu_percent:6.1f}% {exec_count:12d} {avg_time:10.6f}s {min_time:10.6f}s {max_time:10.6f}s")

    def analyze_interrupts(self):
        """Analyze interrupt statistics"""
        print("\n=== Interrupt Analysis ===")

        isr_count = 0
        isr_total_time = 0
        isr_start_time = None
        isr_durations = []

        for event in self.events:
            if event.event_name == "ISR_ENTER":
                isr_start_time = event.timestamp
                isr_count += 1
            elif event.event_name == "ISR_EXIT" and isr_start_time is not None:
                duration = event.timestamp - isr_start_time
                isr_total_time += duration
                isr_durations.append(duration)
                isr_start_time = None

        if isr_count == 0:
            print("No interrupts recorded")
            return

        print(f"Total interrupts: {isr_count}")

        if isr_durations:
            avg_duration = isr_total_time / len(isr_durations)
            min_duration = min(isr_durations)
            max_duration = max(isr_durations)

            print(f"Average ISR duration: {self.parser.timestamp_to_seconds(avg_duration):.6f}s")
            print(f"Min ISR duration: {self.parser.timestamp_to_seconds(min_duration):.6f}s")
            print(f"Max ISR duration: {self.parser.timestamp_to_seconds(max_duration):.6f}s")
            print(f"Total ISR time: {self.parser.timestamp_to_seconds(isr_total_time):.6f}s")

    def print_timeline(self, max_events: int = 100):
        """Print event timeline"""
        print("\n=== Event Timeline ===")
        print(f"Showing first {max_events} events:\n")

        for _i, event in enumerate(self.events[:max_events]):
            time_sec = self.parser.timestamp_to_seconds(event.timestamp)

            if event.event_name in ["TASK_SWITCHED_IN", "TASK_SWITCHED_OUT"]:
                task_name = self.parser.get_task_name(event.handle)
                print(f"{time_sec:12.6f}s: {event.event_name:25s} {task_name}")
            else:
                print(f"{time_sec:12.6f}s: {event.event_name:25s} handle=0x{event.handle:08X}")

    def export_json(self, output_file: Path):
        """Export trace data to JSON"""
        print(f"\nExporting to JSON: {output_file}")

        data = {
            "metadata": {
                "total_events": len(self.events),
                "cpu_frequency": self.parser.cpu_frequency,
                "task_registry": {str(k): v for k, v in self.parser.task_registry.items()},
            },
            "events": [],
        }

        for event in self.events:
            data["events"].append(
                {
                    "type": event.event_name,
                    "timestamp": event.timestamp,
                    "time_seconds": self.parser.timestamp_to_seconds(event.timestamp),
                    "handle": event.handle,
                    "data": event.data,
                }
            )

        with open(output_file, "w") as f:
            json.dump(data, f, indent=2)

        print(f"Exported {len(self.events)} events")

    def export_chrome_trace(self, output_file: Path):
        """Export trace data to Chrome Trace Format (compatible with chrome://tracing and Perfetto)"""
        print(f"\nExporting to Chrome Trace format: {output_file}")

        trace_events = []

        # Track active tasks and ISRs for proper begin/end pairing
        active_tasks = {}  # task_handle -> start_event
        active_isrs = []  # stack of ISR start timestamps
        memory_allocations = {}  # address -> size for tracking memory
        total_allocated = 0  # Running total of allocated memory (may be inaccurate if tracing started mid-execution)
        untracked_frees = 0  # Count of free events for untracked allocations

        for event in self.events:
            timestamp_us = self.parser.timestamp_to_seconds(event.timestamp) * 1_000_000  # Convert to microseconds

            # Task switch events - use duration events
            if event.event_name == "TASK_SWITCHED_IN":
                task_name = self.parser.get_task_name(event.handle)
                active_tasks[event.handle] = {"name": task_name, "ts": timestamp_us}

            elif event.event_name == "TASK_SWITCHED_OUT":
                if event.handle in active_tasks:
                    start_info = active_tasks[event.handle]
                    duration = timestamp_us - start_info["ts"]

                    # Complete event (duration event)
                    trace_events.append(
                        {
                            "name": start_info["name"],
                            "cat": "task",
                            "ph": "X",  # Complete event
                            "ts": start_info["ts"],
                            "dur": duration,
                            "pid": 0,
                            "tid": event.handle,
                            "args": {"handle": f"0x{event.handle:08X}"},
                        }
                    )
                    del active_tasks[event.handle]

            # ISR events - use duration events
            elif event.event_name == "ISR_ENTER":
                active_isrs.append(timestamp_us)

            elif event.event_name == "ISR_EXIT":
                if active_isrs:
                    start_ts = active_isrs.pop()
                    duration = timestamp_us - start_ts

                    trace_events.append({"name": "ISR", "cat": "interrupt", "ph": "X", "ts": start_ts, "dur": duration, "pid": 0, "tid": CHROME_TRACE_ISR_THREAD_ID, "args": {}})

            # Task lifecycle events - instant events
            elif event.event_name == "TASK_CREATE":
                task_name = self.parser.get_task_name(event.handle)
                trace_events.append(
                    {
                        "name": f"Create: {task_name}",
                        "cat": "task_lifecycle",
                        "ph": "i",  # Instant event
                        "s": "g",  # Global scope
                        "ts": timestamp_us,
                        "pid": 0,
                        "tid": event.handle,
                        "args": {"handle": f"0x{event.handle:08X}"},
                    }
                )

            elif event.event_name == "TASK_DELETE":
                task_name = self.parser.get_task_name(event.handle)
                trace_events.append(
                    {
                        "name": f"Delete: {task_name}",
                        "cat": "task_lifecycle",
                        "ph": "i",
                        "s": "g",
                        "ts": timestamp_us,
                        "pid": 0,
                        "tid": event.handle,
                        "args": {"handle": f"0x{event.handle:08X}"},
                    }
                )

            # Task state change events - instant events
            elif event.event_name == "TASK_READY":
                task_name = self.parser.get_task_name(event.handle)
                trace_events.append(
                    {
                        "name": f"Ready: {task_name}",
                        "cat": "task_state",
                        "ph": "i",  # Instant event
                        "s": "t",  # Thread scope
                        "ts": timestamp_us,
                        "pid": 0,
                        "tid": event.handle,
                        "args": {"state": "ready", "handle": f"0x{event.handle:08X}"},
                    }
                )

            elif event.event_name == "TASK_SUSPENDED":
                task_name = self.parser.get_task_name(event.handle)
                trace_events.append(
                    {
                        "name": f"Suspended: {task_name}",
                        "cat": "task_state",
                        "ph": "i",
                        "s": "t",
                        "ts": timestamp_us,
                        "pid": 0,
                        "tid": event.handle,
                        "args": {"state": "suspended", "handle": f"0x{event.handle:08X}"},
                    }
                )

            elif event.event_name == "TASK_RESUMED":
                task_name = self.parser.get_task_name(event.handle)
                from_isr = " (from ISR)" if event.data == 1 else ""
                trace_events.append(
                    {
                        "name": f"Resumed: {task_name}{from_isr}",
                        "cat": "task_state",
                        "ph": "i",
                        "s": "t",
                        "ts": timestamp_us,
                        "pid": 0,
                        "tid": event.handle,
                        "args": {"state": "resumed", "from_isr": bool(event.data), "handle": f"0x{event.handle:08X}"},
                    }
                )

            # Queue events - instant events
            elif event.event_name in ["QUEUE_SEND", "QUEUE_RECEIVE", "QUEUE_CREATE"]:
                trace_events.append(
                    {
                        "name": event.event_name.replace("_", " ").title(),
                        "cat": "queue",
                        "ph": "i",
                        "s": "p",  # Process scope
                        "ts": timestamp_us,
                        "pid": 0,
                        "tid": 0,
                        "args": {"queue": f"0x{event.handle:08X}"},
                    }
                )

            # Semaphore/Mutex events - instant events
            elif event.event_name in ["SEMAPHORE_CREATE", "SEMAPHORE_GIVE", "SEMAPHORE_TAKE", "MUTEX_CREATE", "MUTEX_GIVE", "MUTEX_TAKE"]:
                trace_events.append(
                    {
                        "name": event.event_name.replace("_", " ").title(),
                        "cat": "sync",
                        "ph": "i",
                        "s": "p",
                        "ts": timestamp_us,
                        "pid": 0,
                        "tid": 0,
                        "args": {"handle": f"0x{event.handle:08X}"},
                    }
                )

            # Memory allocation events - counter events
            elif event.event_name == "MALLOC":
                size = event.data
                address = event.handle

                # Validate size
                if size <= 0:
                    print(f"Warning: Invalid malloc size {size} at address 0x{address:08X}, skipping counter update", file=sys.stderr)
                    size = 0  # Still record the event but don't update counter
                else:
                    # Check for double allocation (same address allocated twice)
                    if address in memory_allocations:
                        print(f"Warning: Address 0x{address:08X} allocated twice without free", file=sys.stderr)

                    memory_allocations[address] = size
                    total_allocated += size

                # Instant event for the allocation
                trace_events.append(
                    {"name": "malloc", "cat": "memory", "ph": "i", "s": "p", "ts": timestamp_us, "pid": 0, "tid": 0, "args": {"address": f"0x{address:08X}", "size": size}}
                )

                # Counter event for total allocated memory (only if size was valid)
                if size > 0:
                    trace_events.append({"name": "Memory Usage", "cat": "memory", "ph": "C", "ts": timestamp_us, "pid": 0, "args": {"bytes": total_allocated}})  # Counter event

            elif event.event_name == "FREE":
                address = event.handle
                was_tracked = address in memory_allocations

                if was_tracked:
                    size = memory_allocations[address]
                    total_allocated -= size
                    del memory_allocations[address]
                else:
                    # If allocation wasn't tracked (e.g., occurred before tracing started),
                    # use the size from data field. If data field is 0 or invalid,
                    # we still record the free event but size will be 0.
                    # Don't update total_allocated to avoid negative counter values.
                    if event.data <= 0:
                        print(f"Warning: Invalid free size {event.data} at address 0x{address:08X}", file=sys.stderr)
                        size = 0
                    else:
                        size = event.data
                    untracked_frees += 1

                # Instant event for the free
                trace_events.append(
                    {"name": "free", "cat": "memory", "ph": "i", "s": "p", "ts": timestamp_us, "pid": 0, "tid": 0, "args": {"address": f"0x{address:08X}", "size": size}}
                )

                # Counter event for total allocated memory (only if we're tracking properly)
                if was_tracked:
                    trace_events.append({"name": "Memory Usage", "cat": "memory", "ph": "C", "ts": timestamp_us, "pid": 0, "args": {"bytes": total_allocated}})

        # Create the final trace object
        trace_data = {
            "traceEvents": trace_events,
            "displayTimeUnit": "ms",
            "metadata": {"cpu_frequency": self.parser.cpu_frequency, "task_registry": {str(k): v for k, v in self.parser.task_registry.items()}, "total_events": len(self.events)},
        }

        with open(output_file, "w") as f:
            json.dump(trace_data, f, indent=2)

        print(f"Exported {len(trace_events)} Chrome Trace events")

        # Warn about untracked frees
        if untracked_frees > 0:
            print(f"Warning: {untracked_frees} free event(s) for untracked allocations")
            print("         Memory counter may be inaccurate if tracing started mid-execution")

        print("View in Chrome: chrome://tracing")
        print("View in Perfetto: https://ui.perfetto.dev/")

    def export_perfetto(self, output_file: Path):
        """Export trace data to Perfetto format (Chrome Trace JSON compatible)

        Note: Perfetto UI natively supports Chrome Trace JSON format.
        This wrapper provides a user-friendly API and allows for future
        Perfetto-specific extensions if needed.
        """
        # Perfetto UI supports Chrome Trace JSON format natively
        self.export_chrome_trace(output_file)


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description="FreeRTOS Trace Analyzer - Analyze RTT trace data")

    parser.add_argument("tracefile", type=Path, help="Trace file to analyze")
    parser.add_argument("--timeline", action="store_true", help="Show event timeline")
    parser.add_argument("--timeline-events", type=int, default=100, help="Number of events to show in timeline (default: 100)")
    parser.add_argument("--stats", action="store_true", help="Show statistics summary")
    parser.add_argument("--task-runtime", action="store_true", help="Analyze task runtime")
    parser.add_argument("--interrupts", action="store_true", help="Analyze interrupts")
    parser.add_argument("--export-json", type=Path, help="Export trace data to JSON file")
    parser.add_argument("--export-chrome-trace", type=Path, help="Export trace data to Chrome Trace format (viewable in chrome://tracing)")
    parser.add_argument("--export-perfetto", type=Path, help="Export trace data to Perfetto format (viewable in https://ui.perfetto.dev/)")
    parser.add_argument("--cpu-freq", type=int, default=120000000, help="CPU frequency in Hz (default: 128000000 for STM32F205)")

    args = parser.parse_args()

    if not args.tracefile.exists():
        print(f"Error: Trace file '{args.tracefile}' not found", file=sys.stderr)
        return 1

    # Parse trace file
    trace_parser = TraceParser(args.tracefile)
    trace_parser.cpu_frequency = args.cpu_freq

    if not trace_parser.parse():
        return 1

    # Analyze trace
    analyzer = TraceAnalyzer(trace_parser)

    # Show requested analysis
    if args.stats or (not args.timeline and not args.task_runtime and not args.interrupts):
        analyzer.print_summary()

    if args.task_runtime:
        analyzer.analyze_task_runtime()

    if args.interrupts:
        analyzer.analyze_interrupts()

    if args.timeline:
        analyzer.print_timeline(max_events=args.timeline_events)

    if args.export_json:
        analyzer.export_json(args.export_json)

    if args.export_chrome_trace:
        analyzer.export_chrome_trace(args.export_chrome_trace)

    if args.export_perfetto:
        analyzer.export_perfetto(args.export_perfetto)

    return 0


if __name__ == "__main__":
    sys.exit(main())
