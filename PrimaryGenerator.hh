#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "globals.hh"

class PrimaryGenerator : public G4VUserPrimaryGeneratorAction {
public:
    PrimaryGenerator(const std::string& particleName, G4double energy);
    ~PrimaryGenerator();

    void GeneratePrimaries(G4Event* event) override;

private:
    G4ParticleGun* fParticleGun;
    std::string fParticleName;
    G4double fEnergy;
};
