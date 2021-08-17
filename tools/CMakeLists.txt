include(ArtMake)

project_variable(TOOLS_DIR tools CONFIG
  DOCSTRING "Location of useful scripts and files for ${PROJECT_NAME}")

add_subdirectory(sqlite/memoryTracker)
add_subdirectory(sqlite/timeTracker)
add_subdirectory(migration)

cet_script(filter-memoryTracker-output
           filter-timeTracker-output
           filter-updated-summary)

# FIXME: We should add all the perl files that could change (and
# therefore change the output of the skeleton generator) as dependencies
# to each target. This is not trivial!
function(genskel TYPE SUFFIX)
  cmake_parse_arguments(GS "" "" "EXTRA_ARGS" ${ARGN})
  string(FIND "${TYPE}" ":" split_loc)
  if (NOT split_loc EQUAL -1)
    string(SUBSTRING "${TYPE}" ${split_loc} -1 TYPEARGS)
    string(SUBSTRING "${TYPE}" 0 ${split_loc} TYPE)
  endif()
  add_custom_command(OUTPUT Test${TYPE}_${SUFFIX}.cc
    COMMAND cetskelgen
    ARGS --force ${GS_EXTRA_ARGS} -A ${TYPE}${TYPEARGS} arttest::Test${TYPE}
    COMMENT "Generating Test${TYPE}_${SUFFIX}.cc with args ${GS_EXTRA_ARGS}"
    VERBATIM
    )
  # We knowingly generate code with unused parameters and missing
  # returns: user is supposed to fill those in when xe implements the module
  # methods.
  set_source_files_properties(Test${TYPE}_${SUFFIX}.cc
    PROPERTIES
    COMPILE_FLAGS "-Wno-unused-parameter -Wno-return-type"
  )
endfunction()

if (CETSKEL)
  # Disabled until we have correct dependencies to avoid intermittent
  # failures when building parallel.
  foreach(type IN ITEMS Analyzer Producer Filter)
    genskel(${type} "module")
    cet_build_plugin(Test${type} art::${type} BASENAME_ONLY NO_INSTALL)
  endforeach()

  foreach(type IN ITEMS FileCatalogMetadataPlugin EmptyEventTimestampPlugin)
    genskel(${type} "plugin")
    cet_build_plugin(Test${type} art::${type} BASENAME_ONLY NO_INSTALL)
  endforeach()

  # Simple service (no header).
  genskel(Service "service")
  cet_build_plugin(TestService art::service BASENAME_ONLY NO_INSTALL)

  # Service interface.
  add_custom_command(OUTPUT TestServiceInterface.cc TestServiceInterface.h
    COMMAND cetskelgen
    ARGS --force -e "void doStuff()" -A ServiceInterface arttest::TestServiceInterface
    COMMENT "Generating Test${TYPE}_${SUFFIX}.cc with args ${GS_EXTRA_ARGS}"
    VERBATIM
    )
  set_source_files_properties(TestServiceInterface.cc
    PROPERTIES
    COMPILE_FLAGS "-Wno-unused-parameter -Wno-return-type"
    GENERATED 1
    )

  # Library containing code for service interface.
  cet_make_library(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/TestServiceInterface.cc
    NO_INSTALL EXPORT_SET Testing LIBRARY_NAME TestServiceInterface)

  # Service interface implementation.
  genskel("ServiceImpl:arttest::TestServiceInterface,tools/TestServiceInterface.h"
    "service"
    )
  cet_build_plugin(TestServiceImpl art::service NO_INSTALL
    LIBRARAIES PRIVATE art_test::TestServiceInterface)
endif()
