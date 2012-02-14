cet_enable_asserts()

# add_definitions(-DART_NO_MIX_PTRVECTOR)

# build Persistency libraries
add_subdirectory (Framework/Art)
add_subdirectory (Framework/Core)
add_subdirectory (Framework/IO/Root)
add_subdirectory (Framework/Principal)
add_subdirectory (Framework/Services/Registry)
add_subdirectory (Integration)
add_subdirectory (Persistency/Provenance)
add_subdirectory (Persistency/RootDB)
add_subdirectory (TestObjects)
add_subdirectory (Version)
add_subdirectory (Utilities)
