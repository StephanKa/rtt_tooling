# Build Guide

## Prerequisites

- CMake ≥ 3.20
- Ninja
- GCC/Clang (host builds) or `arm-none-eabi-gcc` (cross-compile)
- Python ≥ 3.8 with dev dependencies (`pip install -e ".[dev]"`)

## CMake Presets

| Preset | Description |
|---|---|
| `default` | Host debug build |
| `release` | Host release build |
| `arm-stm32f205` | ARM cross-compile debug (STM32F205) |
| `arm-stm32f205-release` | ARM cross-compile release (STM32F205) |
| `testing` | Host debug build with unit tests enabled |

## Build Commands

```bash
# Configure
cmake --preset <preset-name>

# Build
cmake --build --preset <preset-name>

# Examples
cmake --preset default && cmake --build --preset default
cmake --preset arm-stm32f205 && cmake --build --preset arm-stm32f205
cmake --preset testing && cmake --build --preset testing
```

Output goes to `build/<preset-name>/`.

## Running Tests

```bash
# Configure and build the testing preset first, then:
ctest --preset testing

# Or via the helper script:
./bin/run_tests.sh
```

## C++ Standard

C++23 is required for all C++ targets. The standard is set by the root CMake project and is not configurable.

## Python Checks

```bash
./bin/run_ruff.sh       # Ruff lint
./bin/run_black.sh      # Black format check
./bin/run_pylint.sh     # Pylint
./bin/run_mypy.sh       # mypy type check
./bin/run_tests.sh      # pytest
./bin/run_all_checks.sh # All of the above
```
