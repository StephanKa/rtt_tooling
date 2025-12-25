#!/bin/bash
# Run mypy type checker on Python scripts

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

echo "Running mypy type checker on Python scripts..."
source .venv/bin/activate
mypy scripts/ tests/ "$@"
