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
  ADD_CUSTOM_COMMAND(OUTPUT ToolsTest${TYPE}_${SUFFIX}.cc
    COMMAND cetskelgen
    ARGS --force ${GS_EXTRA_ARGS} -A ${TYPE}${TYPEARGS} arttest::ToolsTest${TYPE}
    COMMENT "Generating ToolsTest${TYPE}_${SUFFIX}.cc with args ${GS_EXTRA_ARGS}"
    VERBATIM
    )
  # We knowingly generate code with unused parameters and missing
  # returns: user is supposed to fill those in when xe implements the module
  # methods.
  SET_SOURCE_FILES_PROPERTIES(ToolsTest${TYPE}_${SUFFIX}.cc
    PROPERTIES
    COMPILE_FLAGS "-Wno-unused-parameter -Wno-return-type"
    GENERATED 1
    )
endfunction()

if (CETSKEL)
  # Disabled until we have correct dependencies to avoid intermittent
  # failures when building parallel.
  foreach(type Analyzer Producer Filter)
    genskel(${type} "module")
    simple_plugin(ToolsTest${type} "module" NO_INSTALL)
  endforeach()

  foreach(type FileCatalogMetadataPlugin EmptyEventTimestampPlugin)
    genskel(${type} "plugin")
    simple_plugin(ToolsTest${type} "plugin" NO_INSTALL art_Framework_Core)
  endforeach()

  # Simple service (no header).
  genskel(Service "service")
  simple_plugin(ToolsTestService "service" NO_INSTALL)

  # Service interface.
  ADD_CUSTOM_COMMAND(OUTPUT ToolsTestServiceInterface.cc
    ToolsTestServiceInterface.h
    COMMAND cetskelgen
    ARGS --force -e "void doStuff()" -A ServiceInterface arttest::ToolsTestServiceInterface
    COMMENT "Generating ToolsTest${TYPE}_${SUFFIX}.cc with args ${GS_EXTRA_ARGS}"
    VERBATIM
    )
  SET_SOURCE_FILES_PROPERTIES(ToolsTestServiceInterface.cc
    PROPERTIES
    COMPILE_FLAGS "-Wno-unused-parameter -Wno-return-type"
    GENERATED 1
    )
  SET_SOURCE_FILES_PROPERTIES(ToolsTestServiceInterface.h
    PROPERTIES
    GENERATED 1
    )

  # Library containing code for service interface.
  art_make_library(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/ToolsTestServiceInterface.cc
    NO_INSTALL)

  # Service interface implementation.
  genskel("ServiceImpl:arttest::ToolsTestServiceInterface,tools/ToolsTestServiceInterface.h"
    "service"
    )
  simple_plugin(ToolsTestServiceImpl "service" NO_INSTALL tools)
endif()
