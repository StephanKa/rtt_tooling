# RTT Tooling — Agent Guidelines

## Architecture

C++ header-only / static-library toolkit for SEGGER RTT on embedded targets (STM32F205).
Each component (`rtt_logger`, `rtt_unittest`, `rtt_freertos_hooks`, `rtt_freertos_trace`,
`rtt_benchmark`, `rtt_memory_dump`, `rtt_data`, `rtt_fault_handler`) is an independent
CMake subdirectory with its own `CMakeLists.txt`, `include/`, `src/`, and optionally
`examples/` and `tests/`.

Python host-side scripts live in `scripts/`; their tests in `tests/`.

## Code Style

- **C++**: C++23 is required. No extensions (`CMAKE_CXX_EXTENSIONS OFF`).
- **Python**: formatted with Black (line-length 180), linted with Ruff + Pylint, type-checked with mypy. See `pyproject.toml` for rules.

## Build and Test

See [build.md](build.md) for all build and test commands.

## Conventions

- External SEGGER RTT source is fetched via CMake `FetchContent` — do not vendor it manually.
- New C++ modules follow the existing pattern: one subdirectory with `include/<name>/`, `src/`, `CMakeLists.txt`, and `README.md`.
- Python scripts target Python ≥ 3.8; avoid syntax or stdlib features unavailable in 3.8.
- All Python changes must pass `ruff`, `black --check`, `pylint`, `mypy`, and `pytest` before commit.
