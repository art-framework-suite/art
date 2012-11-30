cet_enable_asserts()

# add_definitions(-DART_NO_MIX_PTRVECTOR)

# Make sure tests have correct environment settings.
include(CetTest)
# If you explicitly include CetTest in a subdirectory, you will need to
# re-initialize the test environment.
cet_test_env("FHICL_FILE_PATH=.")

# build Persistency libraries
add_subdirectory (Framework/Art)
add_subdirectory (Framework/Core)
add_subdirectory (Framework/IO/Root)
add_subdirectory (Framework/Principal)
add_subdirectory (Framework/Services/Optional)
add_subdirectory (Framework/Services/Registry)
add_subdirectory (Integration)
add_subdirectory (Persistency/Provenance)
add_subdirectory (Persistency/RootDB)
add_subdirectory (TestObjects)
add_subdirectory (Version)
add_subdirectory (Utilities)

if(HAVE_TBB)
  add_subdirectory(tbb)
endif()
