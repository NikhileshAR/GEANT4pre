#ifndef PrimaryGenerator_h
#define PrimaryGenerator_h

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "globals.hh"
#include <string>

class PrimaryGenerator : public G4VUserPrimaryGeneratorAction {
public:
    PrimaryGenerator(const std::string& particleName, G4double energy);
    ~PrimaryGenerator();

    void GeneratePrimaries(G4Event* event) override;

    // Called between runs to update particle type and energy
    void SetParticle(const std::string& particleName, G4double energy);

private:
    G4ParticleGun* fParticleGun;
    std::string    fParticleName;
    G4double       fEnergy;
};

#endif
