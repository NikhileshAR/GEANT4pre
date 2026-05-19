#ifndef SteppingAction_h
#define SteppingAction_h

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class G4Step;

class SteppingAction : public G4UserSteppingAction {
public:
    // Added runID parameter so each event knows which run it belongs to
    SteppingAction(G4double angleThreshold, G4int runID);
    virtual ~SteppingAction() = default;

    virtual void UserSteppingAction(const G4Step* step) override;

    G4int GetLargeAngleCount() const;

private:
    G4double fAngleThreshold;
    G4int    fLargeAngleCount;
    G4int    fRunID;           // new: run identifier
};

#endif
