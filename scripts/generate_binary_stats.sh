#!/bin/bash
# Generate binary statistics for ARM GCC builds
# Usage: ./generate_binary_stats.sh <build_directory> <build_type>

set -e

BUILD_DIR="${1:?Build directory is required}"
BUILD_TYPE="${2:?Build type is required}"

echo "=== Binary Size Statistics (ARM ${BUILD_TYPE} Build) ==="
echo ""

# Library Files
echo "## Library Files"
find "${BUILD_DIR}" -name "*.a" -exec sh -c '
    echo ""
    echo "### $0"
    arm-none-eabi-size "$0" 2>/dev/null || true
' {} \;

echo ""

# Example Executables
echo "## Example Executables"
find "${BUILD_DIR}" -type f -name "*example" -exec sh -c '
    echo ""
    echo "### $0"
    arm-none-eabi-size "$0" 2>/dev/null || true
    echo ""
    echo "Section details:"
    arm-none-eabi-size -A "$0" 2>/dev/null || true
' {} \;

echo ""

# Summary
echo "## Summary"
echo "Total size of all libraries:"
find "${BUILD_DIR}" -name "*.a" -exec arm-none-eabi-size {} 2>/dev/null \; | \
    awk 'NR>1 {text+=$1; data+=$2; bss+=$3} END {
        printf "   text    data     bss     dec     hex\n"
        printf "%7d %7d %7d %7d %7x\n", text, data, bss, text+data+bss, text+data+bss
    }'
