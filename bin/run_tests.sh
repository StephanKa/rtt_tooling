#!/bin/bash
# Run pytest on test suite

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

echo "Running pytest on test suite..."
source .venv/bin/activate
pytest "$@"
