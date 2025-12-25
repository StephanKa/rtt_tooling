#!/bin/bash
# Run pylint on Python scripts

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

echo "Running pylint on Python scripts..."
source .venv/Scripts/activate
pylint scripts/ tests/ "$@"
