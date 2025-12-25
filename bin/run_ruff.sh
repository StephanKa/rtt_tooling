#!/bin/bash
# Run ruff linter on Python scripts

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

echo "Running ruff linter on Python scripts..."
source .venv/Scripts/activate
ruff check scripts/ tests/ "$@"
