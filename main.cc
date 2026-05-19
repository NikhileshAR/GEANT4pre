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
    std::vector<std::string> particles = {"e-", "mu-"};
    std::vector<G4double>    energies  = {100, 200, 500, 1000, 2000, 5000};
    std::vector<G4double>    thicknessFractions = {0.01, 0.05, 0.1, 0.2, 0.35, 0.5};

    G4int    nEvents      = 10000;
    G4double thresholdDeg = 5.0;
    G4double thresholdRad = thresholdDeg * deg;

    G4double X0_Al = G4NistManager::Instance()
                         ->FindOrBuildMaterial("G4_Al")
                         ->GetRadlen();

    std::ofstream csvLog("params.csv");
    csvLog << "run_id,particle,energy_MeV,thickness_X0,thickness_mm,"
           << "large_angle_prob,output_file\n";

    // ==============================
    // Create RunManager ONCE outside loop
    // ==============================

    auto runManager = new G4RunManager();
    runManager->SetVerboseLevel(0);

    // Step 1: set detector
    auto detector = new DetectorConstruction(0.1 * X0_Al);
    runManager->SetUserInitialization(detector);

    // Step 2: set physics — MUST happen before constructing any user action
    runManager->SetUserInitialization(new PhysicsList());

    // Step 3: construct actions AFTER physics is registered (Geant4-11 requirement)
    auto primaryGen     = new PrimaryGenerator("e-", 100 * MeV);
    auto steppingAction = new SteppingAction(thresholdRad, 0);

    runManager->SetUserAction(primaryGen);
    runManager->SetUserAction(steppingAction);

    // Step 4: initialize
    runManager->Initialize();

    // ==============================
    // Batch loop
    // ==============================

    int runID = 0;

    for (const auto& particle : particles)
    {
        for (G4double energyMeV : energies)
        {
            for (G4double tFrac : thicknessFractions)
            {
                G4double targetThickness = tFrac * X0_Al;

                int tPercent = static_cast<int>(tFrac * 100);
                int eMeV     = static_cast<int>(energyMeV);
                std::string ptag = (particle == "e-") ? "em" : "mu";

                std::ostringstream fname;
                fname << "output_" << ptag << "_"
                      << eMeV << "MeV_"
                      << tPercent << "X0.root";
                std::string outputFile = fname.str();

                std::cout << "\n========================================\n";
                std::cout << "RUN " << runID
                          << " | particle=" << particle
                          << " | energy="   << energyMeV << " MeV"
                          << " | thickness=" << tFrac    << " X0"
                          << " | output="   << outputFile << "\n";
                std::cout << "========================================\n";

                // Update geometry and reinitialise
                detector->thickness = targetThickness;
                runManager->ReinitializeGeometry();
                runManager->PhysicsHasBeenModified();

                // Update particle gun
                primaryGen->SetParticle(particle, energyMeV * MeV);

                // Reset stepping action counters
                steppingAction->Reset(runID);

                // Analysis manager: clear previous run's histos/ntuples
                auto am = G4AnalysisManager::Instance();
                am->Clear();
                am->SetDefaultFileType("root");
                am->SetVerboseLevel(0);

                am->CreateH1("ScatteringAngle_Full",
                             "Scattering Angle (Full);Theta (deg);Counts",
                             180, 0, 180);
                am->CreateH1("ScatteringAngle",
                             "Scattering Angle;Theta (deg);Probability",
                             200, 0, 60);
                am->CreateNtuple("scattering", "Scattering Data");
                am->CreateNtupleDColumn("theta");
                am->CreateNtupleDColumn("energy");
                am->CreateNtupleIColumn("run_id");
                am->FinishNtuple();
                am->OpenFile(outputFile);

                runManager->BeamOn(nEvents);

                G4double largeAngleProb =
                    (double)steppingAction->GetLargeAngleCount() / nEvents;

                std::cout << "Large angle prob (>" << thresholdDeg << " deg): "
                          << largeAngleProb << "\n";

                am->ScaleH1(0, 1.0 / nEvents);
                am->ScaleH1(1, 1.0 / nEvents);
                am->Write();
                am->CloseFile();

                csvLog << runID << "," << particle << ","
                       << energyMeV << "," << tFrac << ","
                       << targetThickness / mm << ","
                       << largeAngleProb << "," << outputFile << "\n";
                csvLog.flush();

                runID++;
            }
        }
    }

    csvLog.close();
    delete runManager;

    std::cout << "\n========================================\n";
    std::cout << "BATCH COMPLETE. Total runs: " << runID << "\n";
    std::cout << "Parameter log: params.csv\n";
    std::cout << "========================================\n";

    return 0;
}