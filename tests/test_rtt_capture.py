"""Unit tests for the unified RTT capture tool."""

import json
from pathlib import Path
from typing import Dict, List, Optional

import pytest
from rtt_capture import RttCapture, parse_channels
from rtt_reader import RttReader


class FakeReader(RttReader):
    """Deterministic in-memory RTT reader."""

    def __init__(self, payloads: Dict[int, List[bytes]]):
        self.payloads = payloads
        self.connected = False

    def connect(self) -> bool:
        self.connected = True
        return True

    def disconnect(self) -> None:
        self.connected = False

    def read_rtt(self, channel: int = 0, size: int = 1024) -> Optional[bytes]:
        del size
        packets = self.payloads.get(channel, [])
        return packets.pop(0) if packets else None

    def is_connected(self) -> bool:
        return self.connected


def test_parse_channels() -> None:
    """Decimal and prefixed channel numbers are accepted."""
    assert parse_channels("0, 1, 0x2") == [0, 1, 2]


@pytest.mark.parametrize("value", ["", "1,1", "-1", "bad"])
def test_parse_channels_rejects_invalid_values(value: str) -> None:
    """Invalid channel lists fail during argument parsing."""
    with pytest.raises(Exception):
        parse_channels(value)


def test_capture_writes_raw_channels_and_metadata(temp_dir: Path) -> None:
    """Each channel is preserved separately and summarized in metadata."""
    fake = FakeReader({0: [b"log"], 1: [b"trace"]})
    capture = RttCapture(
        backend="jlink",
        channels=[0, 1],
        output_dir=temp_dir,
        duration=0.001,
        poll_interval=0,
        reader_factory=lambda _channel: fake,
    )

    stats = capture.capture()

    assert (temp_dir / "channel-0.bin").read_bytes() == b"log"
    assert (temp_dir / "channel-1.bin").read_bytes() == b"trace"
    assert stats[0].bytes_captured == 3
    assert stats[1].bytes_captured == 5
    metadata = json.loads((temp_dir / "metadata.json").read_text(encoding="utf-8"))
    assert metadata["schema_version"] == 1
    assert metadata["channels"][1]["bytes_captured"] == 5
    assert not fake.connected


def test_capture_rejects_duplicate_channels(temp_dir: Path) -> None:
    """Duplicate channels would otherwise overwrite the same output file."""
    with pytest.raises(ValueError, match="unique"):
        RttCapture("jlink", [1, 1], temp_dir)
