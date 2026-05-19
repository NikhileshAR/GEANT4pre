#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
    G4double thickness;
    DetectorConstruction(G4double t) : thickness(t) {}
    G4VPhysicalVolume* Construct() override;
};
