#include "DetectorConstruction.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"

G4VPhysicalVolume* DetectorConstruction::Construct() {
    auto nist = G4NistManager::Instance();

    // World
    G4double worldSize = 1.0*m;
    auto worldSolid = new G4Box("World", worldSize/2, worldSize/2, worldSize/2);
    auto worldMat = nist->FindOrBuildMaterial("G4_AIR");
    auto worldLogic = new G4LogicalVolume(worldSolid, worldMat, "WorldLogic");
    auto worldPhys = new G4PVPlacement(0, G4ThreeVector(), worldLogic, "WorldPhys", 0, false, 0);

    // Aluminium target
    auto alMat = nist->FindOrBuildMaterial("G4_Al");
    auto targetSolid = new G4Box("AlTarget", 5*cm, 5*cm, thickness/2);
    auto targetLogic = new G4LogicalVolume(targetSolid, alMat, "TargetLogic");
    new G4PVPlacement(0, G4ThreeVector(0,0,0), targetLogic, "TargetPhys", worldLogic, false, 0);

    auto screenSolid = new G4Box("Screen", 20*cm, 20*cm, 0.1*mm);
    auto screenLogic = new G4LogicalVolume(screenSolid, worldMat, "ScreenLogic");
    new G4PVPlacement(0, G4ThreeVector(0,0,10*cm),
                      screenLogic, "ScreenPhys",
                      worldLogic, false, 0);

    return worldPhys;
}
