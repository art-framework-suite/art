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

find_package(Boost COMPONENTS date_time graph program_options regex REQUIRED PUBLIC)
find_package(Boost PRIVATE COMPONENTS filesystem headers thread unit_test_framework json REQUIRED)
find_package(CLHEP COMPONENTS Random Vector REQUIRED PUBLIC)
find_package(Range PRIVATE REQUIRED)
find_package(TBB REQUIRED PUBLIC)
find_package(canvas REQUIRED PUBLIC)
find_package(cetlib REQUIRED PUBLIC)
find_package(cetlib_except REQUIRED PUBLIC)
find_package(fhiclcpp REQUIRED PUBLIC)
find_package(hep_concurrency REQUIRED PUBLIC)
find_package(messagefacility REQUIRED PUBLIC)

# Register export sets here in dependency order to avoid issues.
cet_register_export_set(SET_NAME ServicePluginType NAMESPACE art_plugin_types)
cet_register_export_set(SET_NAME Services NAMESPACE art)
cet_register_export_set(SET_NAME ServiceUsers NAMESPACE art)
cet_register_export_set(SET_NAME PluginSupport NAMESPACE art_plugin_support)
cet_register_export_set(SET_NAME PluginTypes NAMESPACE art_plugin_types)

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
