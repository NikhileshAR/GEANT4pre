#include "PhysicsList.hh"
#include "G4EmStandardPhysics_option4.hh"

PhysicsList::PhysicsList() {
    RegisterPhysics(new G4EmStandardPhysics_option4());
}