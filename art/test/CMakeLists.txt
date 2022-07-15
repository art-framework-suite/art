include(CetTest)
cet_enable_asserts()
cet_test_env("FHICL_FILE_PATH=.")

find_package(Boost COMPONENTS filesystem graph REQUIRED)
find_package(Catch2 REQUIRED)
find_package(canvas REQUIRED EXPORT)
find_package(cetlib REQUIRED)
find_package(fhiclcpp REQUIRED)
find_package(hep_concurrency REQUIRED)
find_package(messagefacility REQUIRED)

# For namespaced aliases and any installed libraries.
cet_register_export_set(SET_NAME Testing NAMESPACE art_test SET_DEFAULT)

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
    ${ARGN}
  )
endfunction()

set(filterOutputModule "${CMAKE_CURRENT_SOURCE_DIR}/filterOutputModule")
########################################################################

include(BuildPlugins)

add_subdirectory(TestObjects)

add_subdirectory(Configuration)
add_subdirectory(Framework/Art)
add_subdirectory(Framework/Core)
add_subdirectory(Framework/EventProcessor)
add_subdirectory(Framework/ExceptionHandling)
add_subdirectory(Framework/IO)
add_subdirectory(Framework/Modules)
add_subdirectory(Framework/Principal)
add_subdirectory(Framework/Services/Optional)
add_subdirectory(Framework/Services/Registry)
add_subdirectory(Framework/Services/System)
add_subdirectory(Persistency/Provenance)
add_subdirectory(Utilities)
add_subdirectory(Version)
