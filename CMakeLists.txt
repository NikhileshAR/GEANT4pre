cmake_minimum_required(VERSION 3.12)
project(CoulombScattering)

find_package(Geant4 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})

# Add all your source files
add_executable(scattering
    main.cc
    DetectorConstruction.cc
    PhysicsList.cc
    PrimaryGenerator.cc
    SteppingAction.cc
)

target_link_libraries(scattering ${Geant4_LIBRARIES})
