#!/bin/bash
# run_script.sh — convenience wrapper for run_batch.sh
# Usage: bash run_script.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
"$SCRIPT_DIR/run_batch.sh"
