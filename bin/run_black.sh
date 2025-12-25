#!/bin/bash
# Run black formatter on Python scripts

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

echo "Running black formatter on Python scripts..."
source .venv/bin/activate
black scripts/ tests/ "$@"
