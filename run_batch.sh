#!/bin/bash
# run_batch.sh — build and run the full Geant4 surrogate data generation batch
# Usage: bash run_batch.sh
# Run from your project root directory (where CMakeLists.txt lives)

set -e  # exit immediately on any error

PROJECT_DIR="$(pwd)"
BUILD_DIR="$PROJECT_DIR/build"
OUTPUT_DIR="$PROJECT_DIR/data"

echo "========================================"
echo " G4Surrogate — Phase 1 Data Generation"
echo "========================================"
echo "Project dir : $PROJECT_DIR"
echo "Build dir   : $BUILD_DIR"
echo "Output dir  : $OUTPUT_DIR"
echo ""

# Create output directory for ROOT files
mkdir -p "$OUTPUT_DIR"

# Build
echo "[1/3] Cleaning and building..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release > cmake.log 2>&1
make -j$(nproc) > make.log 2>&1
echo "      Build successful."

# Run
echo "[2/3] Running batch simulation..."
cd "$OUTPUT_DIR"
"$BUILD_DIR/scattering"

# Move CSV log to output dir (it's generated in cwd)
echo "[3/3] Done. Output files:"
ls -lh "$OUTPUT_DIR"/*.root 2>/dev/null | wc -l | xargs -I{} echo "      {} ROOT files generated"
echo "      Parameter log: $OUTPUT_DIR/params.csv"
echo ""
echo "========================================"
echo " Batch complete. Pull data to your ML  "
echo " environment for Phase 2.              "
echo "========================================"
