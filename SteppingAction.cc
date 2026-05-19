#include "SteppingAction.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

#include <cmath>

SteppingAction::SteppingAction(G4double angleThreshold, G4int runID)
: fAngleThreshold(angleThreshold),
  fLargeAngleCount(0),
  fRunID(runID)
{}

G4int SteppingAction::GetLargeAngleCount() const {
    return fLargeAngleCount;
}

void SteppingAction::Reset(G4int newRunID) {
    fLargeAngleCount = 0;
    fRunID           = newRunID;
}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    auto track = step->GetTrack();
    if (track->GetTrackID() != 1) return;

    auto pre  = step->GetPreStepPoint();
    auto post = step->GetPostStepPoint();
    if (!pre || !post) return;

    if (pre->GetPhysicalVolume()->GetName() == "TargetPhys" &&
        (post->GetPhysicalVolume() == nullptr ||
         post->GetPhysicalVolume()->GetName() != "TargetPhys"))
    {
        G4ThreeVector beamDir(0, 0, 1);
        auto p_out = post->GetMomentumDirection();

        double cosTheta = beamDir.dot(p_out);
        if (cosTheta >  1.0) cosTheta =  1.0;
        if (cosTheta < -1.0) cosTheta = -1.0;

        double thetaRad  = std::acos(cosTheta);
        double thetaDeg  = thetaRad / CLHEP::deg;
        double energyMeV = post->GetKineticEnergy() / CLHEP::MeV;

        auto am = G4AnalysisManager::Instance();
        am->FillH1(0, thetaDeg);
        am->FillH1(1, thetaDeg);
        am->FillNtupleDColumn(0, thetaDeg);
        am->FillNtupleDColumn(1, energyMeV);
        am->FillNtupleIColumn(2, fRunID);
        am->AddNtupleRow();

        if (thetaRad > fAngleThreshold)
            fLargeAngleCount++;
    }
}