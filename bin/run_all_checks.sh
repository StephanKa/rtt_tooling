#!/bin/bash
# Run all static analysis tools

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

echo "=========================================="
echo "Running all static analysis checks"
echo "=========================================="
echo

# Format with black
echo "1. Running black formatter..."
source .venv/Scripts/activate
black --check scripts/ tests/
echo "✓ black passed"
echo

# Run ruff
echo "2. Running ruff linter..."
ruff check scripts/ tests/
echo "✓ ruff passed"
echo

# Run pylint
echo "3. Running pylint..."
pylint scripts/ tests/
echo "✓ pylint passed"
echo

# Run mypy
echo "4. Running mypy type checker..."
mypy scripts/ tests/
echo "✓ mypy passed"
echo

# Run tests
echo "5. Running pytest..."
pytest
echo "✓ pytest passed"
echo

echo "=========================================="
echo "All checks passed!"
echo "=========================================="
