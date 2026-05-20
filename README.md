# GEANT4pre

## Prerequisites
- Geant4 installed and discoverable by CMake (set `Geant4_DIR` or `CMAKE_PREFIX_PATH`).

## Quick start (build + run batch)
From the repo root:

```
bash run_batch.sh
```

Or:

```
bash run_script.sh
```

Outputs are written to `data/`:
- `output_*.root` (ROOT files)
- `params.csv` (run metadata)

## Manual build/run (optional)
```
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
mkdir -p data
cd data
../build/scattering
```

## Inspecting ROOT output (example)
```
root data/output_em_100MeV_t1000X0.root
ScatteringAngle_Full->Draw();
gPad->SetLogy();
ScatteringAngle->Draw();
gPad->SetLogy();
TTree* t = (TTree*)_file0->Get("scattering");
t->Draw("theta");
```
