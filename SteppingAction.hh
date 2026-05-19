#ifndef SteppingAction_h
#define SteppingAction_h

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class G4Step;

class SteppingAction : public G4UserSteppingAction {
public:
    SteppingAction(G4double angleThreshold, G4int runID);
    virtual ~SteppingAction() = default;

    virtual void UserSteppingAction(const G4Step* step) override;

    G4int GetLargeAngleCount() const;

    // Called between runs to reset counter and update run ID
    void Reset(G4int newRunID);

private:
    G4double fAngleThreshold;
    G4int    fLargeAngleCount;
    G4int    fRunID;
};

#endif
