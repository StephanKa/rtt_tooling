"""Shared test utilities and fixtures for RTT tools tests."""

import tempfile
from pathlib import Path
from typing import Generator

import pytest


@pytest.fixture
def temp_dir() -> Generator[Path, None, None]:
    """Create a temporary directory for tests."""
    with tempfile.TemporaryDirectory() as tmpdir:
        yield Path(tmpdir)


@pytest.fixture
def sample_log_file(temp_dir: Path) -> Path:
    """Create a sample log file for testing."""
    log_file = temp_dir / "test.log"
    log_file.write_text(
        """[INFO] Application started
[DEBUG] Initializing system
[WARN] Low memory warning
[ERROR] Connection failed
[INFO] Retrying connection
[INFO] Connection successful
"""
    )
    return log_file


@pytest.fixture
def sample_binary_file(temp_dir: Path) -> Path:
    """Create a sample binary file for testing."""
    binary_file = temp_dir / "test.bin"
    # Write some test binary data
    binary_file.write_bytes(b"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09")
    return binary_file
