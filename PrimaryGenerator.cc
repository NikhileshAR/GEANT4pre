#include "PrimaryGenerator.hh"
#include "G4ParticleTable.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"

PrimaryGenerator::PrimaryGenerator(const std::string& particleName,
                                   G4double energy)
: fParticleName(particleName),
  fEnergy(energy)
{
    fParticleGun = new G4ParticleGun(1);
}

PrimaryGenerator::~PrimaryGenerator()
{
    delete fParticleGun;
}

void PrimaryGenerator::GeneratePrimaries(G4Event* event)
{
    auto particleTable = G4ParticleTable::GetParticleTable();
    auto particle = particleTable->FindParticle(fParticleName);

    if(!particle)
    {
        G4Exception("PrimaryGenerator",
                    "InvalidParticle",
                    FatalException,
                    "Particle not found. Use e- or mu-.");
    }

    fParticleGun->SetParticleDefinition(particle);
    fParticleGun->SetParticleEnergy(fEnergy);
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0,0,1));
    fParticleGun->SetParticlePosition(G4ThreeVector(0,0,-0.5*m));

    fParticleGun->GeneratePrimaryVertex(event);
}
