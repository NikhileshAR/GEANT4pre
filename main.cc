/*
 * main.cc — G4Surrogate Phase 1 Data Generation (v2, clean)
 *
 * Grid design:
 *   Particles  : e-, mu-
 *   Energies   : 20 log-spaced points, 100–5000 MeV
 *   Thicknesses: 12 log-spaced points, 0.01–0.50 X0
 *   Total runs : 20 × 12 × 2 = 480
 *   Events/run : 10,000
 *
 * Fixes vs v1:
 *   - Grid is truly log-spaced on both axes (was uneven, especially thickness)
 *   - Increased grid density to 480 runs for better coverage
 *   - Filename encodes thickness as fraction string (e.g. "0.0100X0") not percent
 *     → extract_features.py can parse it unambiguously without dividing by 100
 *   - large_angle_prob threshold is 5 deg consistently in both C++ and Python
 *   - params.csv writes thickness_X0 as fraction (was fraction, stays fraction — no change)
 *   - RunManager created once outside loop (segfault fix from v1 preserved)
 *   - Output files go to cwd (run from data/ directory via run_batch.sh)
 *
 * Build:  see CMakeLists.txt (unchanged)
 * Run:    bash run_batch.sh   (from project root)
 */

#include "G4RunManager.hh"
#include "G4AnalysisManager.hh"
#include "G4NistManager.hh"

#include "DetectorConstruction.hh"
#include "PrimaryGenerator.hh"
#include "PhysicsList.hh"
#include "SteppingAction.hh"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <string>
#include <vector>

#include "G4SystemOfUnits.hh"
#include "CLHEP/Units/SystemOfUnits.h"

using namespace CLHEP;

std::vector<G4double> LogSpace(G4double min, G4double max, int count)
{
    std::vector<G4double> values;
    values.reserve(count);

    const G4double logMin = std::log10(min);
    const G4double logMax = std::log10(max);
    const G4double step   = (logMax - logMin) / static_cast<G4double>(count - 1);

    for (int i = 0; i < count; ++i)
    {
        values.push_back(std::pow(10.0, logMin + step * i));
    }

    return values;
}

int main()
{
    // ══════════════════════════════════════════════════════════════════
    // Grid definition — log-spaced on both axes
    // ══════════════════════════════════════════════════════════════════

    std::vector<std::string> particles = {"e-", "mu-"};

    const int energyPoints    = 20;
    const int thicknessPoints = 12;

    // 20 log-spaced energies: 100 → 5000 MeV
    std::vector<G4double> energies = LogSpace(100.0, 5000.0, energyPoints);

    // 12 log-spaced thicknesses: 0.01 → 0.50 X0 (fraction of radiation length)
    std::vector<G4double> thicknessFractions =
        LogSpace(0.01, 0.50, thicknessPoints);

    // ══════════════════════════════════════════════════════════════════
    // Run settings
    // ══════════════════════════════════════════════════════════════════

    const G4int    nEvents        = 10000;
    const G4double thresholdDeg   = 5.0;          // large-angle threshold — degrees
    const G4double thresholdRad   = thresholdDeg * deg;

    G4double X0_Al = G4NistManager::Instance()
                         ->FindOrBuildMaterial("G4_Al")
                         ->GetRadlen();

    // ══════════════════════════════════════════════════════════════════
    // CSV log header
    // Columns match what extract_features.py expects exactly:
    //   run_id, particle, energy_MeV, thickness_X0 (fraction), thickness_mm,
    //   large_angle_prob (threshold = 5 deg), output_file
    // ══════════════════════════════════════════════════════════════════

    std::ofstream csvLog("params.csv");
    csvLog << "run_id,particle,energy_MeV,thickness_X0,thickness_mm,"
           << "large_angle_prob,output_file\n";

    // ══════════════════════════════════════════════════════════════════
    // RunManager — created ONCE outside loop (segfault fix)
    // ══════════════════════════════════════════════════════════════════

    auto runManager = new G4RunManager();
    runManager->SetVerboseLevel(0);

    // Geant4-11 requirement: detector + physics MUST be registered
    // before any user action class is constructed or registered.
    auto detector = new DetectorConstruction(0.1 * X0_Al);
    runManager->SetUserInitialization(detector);
    runManager->SetUserInitialization(new PhysicsList());  // ← physics first

    // Only now safe to construct user actions
    auto primaryGen     = new PrimaryGenerator("e-", 100 * MeV);
    auto steppingAction = new SteppingAction(thresholdRad, 0);
    runManager->SetUserAction(primaryGen);
    runManager->SetUserAction(steppingAction);
    runManager->Initialize();

    // ══════════════════════════════════════════════════════════════════
    // Batch loop
    // ══════════════════════════════════════════════════════════════════

    int runID = 0;

    for (const auto& particle : particles)
    {
        for (G4double energyMeVRaw : energies)
        {
            const G4double energyMeV = std::round(energyMeVRaw);
            for (G4double tFrac : thicknessFractions)
            {
                G4double targetThickness = tFrac * X0_Al;

                // ── Build output filename ─────────────────────────────
                // Format: output_em_100MeV_t1000X0.root
                // thickness encoded as integer = round(tFrac * 100000)
                // e.g. 0.01 -> t1000,  0.5 -> t50000
                // NO dots in filename: ROOT splits on last dot to detect
                // file type; a second dot causes silent write failure.
                // extract_features.py recovers: tFrac = tCode / 100000.0
                std::string ptag = (particle == "e-") ? "em" : "mu";
                int eMeV  = static_cast<int>(energyMeV);
                int tCode = static_cast<int>(std::round(tFrac * 100000));

                std::ostringstream fname;
                fname << "output_" << ptag << "_"
                      << eMeV << "MeV_"
                      << "t" << tCode << "X0.root";
                std::string outputFile = fname.str();

                std::cout << "\n========================================\n"
                          << "RUN "     << runID
                          << " | "      << particle
                          << " | "      << eMeV      << " MeV"
                          << " | "      << tFrac     << " X0"
                          << " | "      << outputFile << "\n"
                          << "========================================\n";

                // ── Update geometry ───────────────────────────────────
                detector->thickness = targetThickness;
                runManager->ReinitializeGeometry();
                runManager->PhysicsHasBeenModified();

                // ── Update beam ───────────────────────────────────────
                primaryGen->SetParticle(particle, energyMeV * MeV);

                // ── Reset stepping action ─────────────────────────────
                steppingAction->Reset(runID);

                // ── Set up analysis manager ───────────────────────────
                auto am = G4AnalysisManager::Instance();
                am->Clear();
                am->SetDefaultFileType("root");
                am->SetVerboseLevel(0);

                // H1: full 0–180 deg range (normalised by nEvents after run)
                am->CreateH1("ScatteringAngle_Full",
                             "Scattering Angle (Full);Theta (deg);Counts",
                             180, 0.0, 180.0);
                // H1: zoomed 0–60 deg range
                am->CreateH1("ScatteringAngle",
                             "Scattering Angle;Theta (deg);Probability",
                             200, 0.0, 60.0);

                // Ntuple — three branches, same as SteppingAction.cc fills
                am->CreateNtuple("scattering", "Scattering Data");
                am->CreateNtupleDColumn("theta");      // col 0: degrees
                am->CreateNtupleDColumn("energy");     // col 1: MeV
                am->CreateNtupleIColumn("run_id");     // col 2: integer
                am->FinishNtuple();
                am->OpenFile(outputFile);

                // ── Run ───────────────────────────────────────────────
                runManager->BeamOn(nEvents);

                // ── Post-run: large-angle probability ─────────────────
                // Threshold: 5 deg (= thresholdDeg above)
                // This MUST match LARGE_ANGLE_DEG in extract_features.py
                G4double largeAngleProb =
                    static_cast<G4double>(steppingAction->GetLargeAngleCount())
                    / static_cast<G4double>(nEvents);

                std::cout << "Large-angle prob (>" << thresholdDeg
                          << " deg): " << largeAngleProb << "\n";

                // ── Write ROOT file ───────────────────────────────────
                am->ScaleH1(0, 1.0 / nEvents);
                am->ScaleH1(1, 1.0 / nEvents);
                am->Write();
                am->CloseFile();

                // ── CSV log ───────────────────────────────────────────
                // thickness_X0 written as fraction (not percent)
                csvLog << runID         << ","
                       << particle      << ","
                       << eMeV          << ","
                       << std::fixed << std::setprecision(6) << tFrac   << ","
                       << std::fixed << std::setprecision(4)
                       << targetThickness / mm << ","
                       << std::fixed << std::setprecision(6) << largeAngleProb << ","
                       << outputFile    << "\n";
                csvLog.flush();

                runID++;
            }
        }
    }

    // ══════════════════════════════════════════════════════════════════
    csvLog.close();
    delete runManager;

    std::cout << "\n========================================\n"
              << "BATCH COMPLETE. Total runs: " << runID << "\n"
              << "Parameter log : params.csv\n"
              << "ROOT files    : " << runID << " × output_*.root\n"
              << "========================================\n";

    return 0;
}
