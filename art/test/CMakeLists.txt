include(CetTest)
cet_enable_asserts()
cet_test_env("FHICL_FILE_PATH=.")

########################################################################
# For use in subdirectories as necessary.

####################################
# config_ref_test
#
# Make reference comparisons easier.

function(config_ref_test TNAME)
  cet_test(${TNAME}_r HANDBUILT
    TEST_EXEC cat
    TEST_ARGS ../${TNAME}_w.d/config.out
    DATAFILES ${TNAME}-ref.txt
    REF ${TNAME}-ref.txt
    REQUIRED_FILES ../${TNAME}_w.d/config.out
    TEST_PROPERTIES DEPENDS ${TNAME}_w
    ${ARGN})
endfunction()

set(filterOutputModule "${CMAKE_CURRENT_SOURCE_DIR}/filterOutputModule")
########################################################################

# build Persistency libraries
add_subdirectory(Configuration)
add_subdirectory(Framework/Art)
add_subdirectory(Framework/Core)
add_subdirectory(Framework/EventProcessor)
add_subdirectory(Framework/IO)
add_subdirectory(Framework/Modules)
add_subdirectory(Framework/Principal)
add_subdirectory(Framework/Services/Optional)
add_subdirectory(Framework/Services/Registry)
add_subdirectory(Framework/Services/System)
add_subdirectory(Framework/Services/Basic)
add_subdirectory(Persistency/Provenance)
add_subdirectory(TestObjects)
add_subdirectory(Utilities)
add_subdirectory(Version)
