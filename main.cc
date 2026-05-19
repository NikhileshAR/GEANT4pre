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
#include <string>
#include <vector>
#include "G4SystemOfUnits.hh"
#include "CLHEP/Units/SystemOfUnits.h"

using namespace CLHEP;

int main()
{
    // ==============================
    // Parameter Grid
    // ==============================

    std::vector<std::string> particles = {"e-", "mu-"};

    // Energies in MeV
    std::vector<G4double> energies = {100, 200, 500, 1000, 2000, 5000};

    // Target thickness as fraction of X0
    std::vector<G4double> thicknessFractions = {0.01, 0.05, 0.1, 0.2, 0.35, 0.5};

    // Events per run
    G4int nEvents = 10000;

    // Large angle threshold (degrees) — fixed for all runs
    G4double thresholdDeg = 5.0;

    // ==============================
    // Material radiation lengths
    // ==============================

    auto nist = G4NistManager::Instance();
    auto alMat = nist->FindOrBuildMaterial("G4_Al");
    G4double X0_Al = alMat->GetRadlen();

    // ==============================
    // CSV log of all runs
    // ==============================

    std::ofstream csvLog("params.csv");
    csvLog << "run_id,particle,energy_MeV,thickness_X0,thickness_mm,"
           << "large_angle_prob,output_file\n";

    int runID = 0;

    // ==============================
    // Batch loop
    // ==============================

    for (const auto& particle : particles)
    {
        for (G4double energyMeV : energies)
        {
            for (G4double tFrac : thicknessFractions)
            {
                G4double targetThickness = tFrac * X0_Al;

                // Build output filename: e.g. output_em_100MeV_010X0.root
                // tFrac * 100 as integer for clean filename
                int tPercent = static_cast<int>(tFrac * 100);
                int eMeV     = static_cast<int>(energyMeV);

                // particle tag: e- -> em, mu- -> mu
                std::string ptag = (particle == "e-") ? "em" : "mu";

                std::ostringstream fname;
                fname << "output_"
                      << ptag << "_"
                      << eMeV << "MeV_"
                      << tPercent << "X0"
                      << ".root";

                std::string outputFile = fname.str();

                std::cout << "\n========================================\n";
                std::cout << "RUN " << runID
                          << " | particle=" << particle
                          << " | energy=" << energyMeV << " MeV"
                          << " | thickness=" << tFrac << " X0"
                          << " | output=" << outputFile << "\n";
                std::cout << "========================================\n";

                // ==============================
                // Analysis Manager setup
                // ==============================

                auto analysisManager = G4AnalysisManager::Instance();
                analysisManager->SetDefaultFileType("root");

                analysisManager->CreateH1(
                    "ScatteringAngle_Full",
                    "Scattering Angle (Full);Theta (deg);Counts",
                    180, 0, 180);

                analysisManager->CreateH1(
                    "ScatteringAngle",
                    "Scattering Angle;Theta (deg);Probability",
                    200, 0, 60);

                // Ntuple: theta, energy, run_id
                analysisManager->CreateNtuple("scattering", "Scattering Data");
                analysisManager->CreateNtupleDColumn("theta");
                analysisManager->CreateNtupleDColumn("energy");
                analysisManager->CreateNtupleIColumn("run_id");
                analysisManager->FinishNtuple();

                analysisManager->OpenFile(outputFile);

                // ==============================
                // Run Manager
                // ==============================

                auto runManager = new G4RunManager();

                runManager->SetUserInitialization(
                    new DetectorConstruction(targetThickness));

                runManager->SetUserInitialization(
                    new PhysicsList());

                runManager->SetUserAction(
                    new PrimaryGenerator(particle, energyMeV * MeV));

                G4double thresholdRad = thresholdDeg * deg;
                auto steppingAction = new SteppingAction(thresholdRad, runID);
                runManager->SetUserAction(steppingAction);

                runManager->Initialize();
                runManager->BeamOn(nEvents);

                // ==============================
                // Results
                // ==============================

                G4int largeCount = steppingAction->GetLargeAngleCount();
                G4double largeAngleProb = (double)largeCount / nEvents;

                std::cout << "Large angle prob (>" << thresholdDeg << " deg): "
                          << largeAngleProb << "\n";

                // Scale histograms to probability
                analysisManager->ScaleH1(0, 1.0 / nEvents);
                analysisManager->ScaleH1(1, 1.0 / nEvents);

                analysisManager->Write();
                analysisManager->CloseFile();

                // Log to CSV
                csvLog << runID << ","
                       << particle << ","
                       << energyMeV << ","
                       << tFrac << ","
                       << targetThickness / mm << ","
                       << largeAngleProb << ","
                       << outputFile << "\n";

                delete runManager;
                runID++;
            }
        }
    }

    csvLog.close();

    std::cout << "\n========================================\n";
    std::cout << "BATCH COMPLETE. Total runs: " << runID << "\n";
    std::cout << "Parameter log saved to params.csv\n";
    std::cout << "========================================\n";

    return 0;
}