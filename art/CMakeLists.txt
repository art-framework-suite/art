include(BasicPlugin)
include(CetMakeCompletions)

set(dest_subdir Modules)

# Used downstream for simple plugin builders.
function(make_simple_builder NAME)
  # Drop namespacing to ascertain target name (art internal convention
  # only).
  string(REGEX REPLACE "^.*::" "" TGT "${NAME}")
  list(POP_FRONT ARGN kw)
  if (kw STREQUAL "BASE")
    list(POP_FRONT ARGN BASE)
    unset(kw)
  elseif (TGT MATCHES "([A-Z][a-z]+)$")
    string(TOLOWER "art::${CMAKE_MATCH_1}" BASE)
  else()
    message(FATAL_ERROR "unable to deduce BASE: specify explicitly")
  endif()
  string(REGEX REPLACE "^(.+)$"
    "$<$<TARGET_EXISTS:art_plugin_types::\\1>:art_plugin_types::\\1>"
    TGT "${TGT}")
  cet_make_plugin_builder(${NAME} ${BASE} ${dest_subdir} ${kw} ${ARGN} LIBRARIES
    REG ${TGT} NOP)
endfunction()

find_package(Boost COMPONENTS date_time graph program_options regex REQUIRED EXPORT)
find_package(Boost COMPONENTS filesystem headers thread unit_test_framework json REQUIRED)
find_package(CLHEP COMPONENTS Random Vector REQUIRED EXPORT)
find_package(Range-v3 REQUIRED)
find_package(TBB REQUIRED EXPORT)
find_package(canvas REQUIRED EXPORT)
find_package(cetlib REQUIRED EXPORT)
find_package(cetlib_except REQUIRED EXPORT)
find_package(fhiclcpp REQUIRED EXPORT)
find_package(hep_concurrency REQUIRED EXPORT)
find_package(messagefacility REQUIRED EXPORT)

add_subdirectory(Version)
add_subdirectory(Utilities)
add_subdirectory(Persistency)
add_subdirectory(Framework)

# Add hand-tooled plugin builders to the list of generated ones.
list(APPEND CETMODULES_PLUGIN_BUILDERS_PROJECT_${CETMODULES_CURRENT_PROJECT_NAME}
  art::plugin art::module art::service art::source art::tool)
# Generate the convenience module to load all the generated and
# hand-tooled plugin builders so we can use them easily in test/.
cet_collect_plugin_builders(Modules ArtPlugins)

# Testing
add_subdirectory(test)
