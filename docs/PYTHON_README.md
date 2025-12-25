# Python Tools for RTT Development

This directory contains Python scripts for working with RTT (Real-Time Transfer) data from embedded devices.

## Installation

The project uses `uv` as the package manager. To set up the development environment:

```bash
# Install uv (if not already installed)
pip install uv

# Create virtual environment
uv venv

# Install dependencies (including dev dependencies)
uv pip install -e ".[dev]"

# Activate virtual environment
source .venv/Scripts/activate  # On Linux/macOS
# or
.venv\Scripts\activate  # On Windows
```

## Scripts

### RTT Analyzer (`rtt_analyzer.py`)
Analyzes RTT log files and provides statistics and filtering capabilities.

```bash
python scripts/rtt_analyzer.py logfile.log
python scripts/rtt_analyzer.py logfile.log --level ERROR
python scripts/rtt_analyzer.py logfile.log --search "connection" --ignore-case
python scripts/rtt_analyzer.py logfile.log --stats-only
```

### RTT Data Reader (`rtt_data_reader.py`)
Reads structured data sent via RTT from embedded devices.

```bash
# Read from OpenOCD
python scripts/rtt_data_reader.py --backend openocd --host localhost --port 4444

# Read from J-Link
python scripts/rtt_data_reader.py --backend jlink --device STM32F205RB

# Parse binary file
python scripts/rtt_data_reader.py --file data.bin
```

### RTT Reader (`rtt_reader.py`)
Host-side RTT reading for OpenOCD and J-Link.

```bash
# Read from J-Link
python scripts/rtt_reader.py --backend jlink --device STM32F205RB

# Read from OpenOCD
python scripts/rtt_reader.py --backend openocd --host localhost --port 4444

# Save output to file
python scripts/rtt_reader.py --backend jlink --device STM32F205RB --output rtt_log.txt
```

### RTT Trace Analyzer (`rtt_trace_analyzer.py`)
Parses and analyzes FreeRTOS trace data captured via RTT.

```bash
python scripts/rtt_trace_analyzer.py trace.bin
python scripts/rtt_trace_analyzer.py trace.bin --timeline
python scripts/rtt_trace_analyzer.py trace.bin --stats
python scripts/rtt_trace_analyzer.py trace.bin --task-runtime
python scripts/rtt_trace_analyzer.py trace.bin --export-chrome-trace trace.json
```

### RTT Trace Reader (`rtt_trace_reader.py`)
Reads FreeRTOS trace data via RTT.

```bash
# Read from J-Link
python scripts/rtt_trace_reader.py -d STM32F205RB -p jlink

# Read from OpenOCD (ST-Link)
python scripts/rtt_trace_reader.py -d stm32f2x -p openocd

# Save to file
python scripts/rtt_trace_reader.py -d STM32F205RB -p jlink -o trace.bin
```

### RTT Viewer (`rtt_viewer.py`)
Connects to SEGGER J-Link and displays RTT output in real-time.

```bash
python scripts/rtt_viewer.py --device STM32F205RB --interface SWD
```

## Development

### Running Static Analyzers

We use multiple static analysis tools to ensure code quality:

```bash
# Run black formatter
./bin/run_black.sh

# Run ruff linter
./bin/run_ruff.sh

# Run pylint
./bin/run_pylint.sh

# Run mypy type checker
./bin/run_mypy.sh

# Run all checks
./bin/run_all_checks.sh
```

### Running Tests

```bash
# Run all tests
./bin/run_tests.sh

# Or use pytest directly
pytest

# Run with coverage
pytest --cov=scripts --cov-report=html
```

### Code Quality Standards

- **Line length**: 180 characters maximum
- **Black**: Code formatting (enforced)
- **Ruff**: Fast linting with comprehensive rule set
- **Pylint**: Additional code quality checks (target: 10/10)
- **Mypy**: Static type checking
- **Test coverage**: Minimum 40% (target: >70%)

### Pre-commit Checks

Before committing code, run:

```bash
./bin/run_all_checks.sh
```

This runs:
1. Black formatting check
2. Ruff linting
3. Pylint code quality
4. Mypy type checking
5. Pytest test suite

## Configuration

All tool configurations are in `pyproject.toml`:
- Black formatting options
- Ruff linting rules
- Pylint settings
- Mypy type checker options
- Pytest configuration

## CI/CD

GitHub Actions automatically runs all checks on push and pull requests. See `.github/workflows/build.yml` for details.

## Dependencies

### Runtime Dependencies
None - scripts use only Python standard library

### Development Dependencies
- pytest: Testing framework
- pytest-cov: Coverage reporting
- black: Code formatter
- ruff: Fast Python linter
- pylint: Code quality checker
- mypy: Static type checker

### Optional Dependencies
- pylink-square: For J-Link RTT support
- telnetlib: For OpenOCD RTT support (deprecated, built-in)
