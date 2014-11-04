# macros for building plugin libraries
#
# The plugin type is expected to be service, source, or module,
# but we do not enforce this.
#
# USAGE:
# simple_plugin( <name> <plugin type>
#                [library list]
#                [USE_BOOST_UNIT]
#                [ALLOW_UNDERSCORES]
#                [BASENAME_ONLY]
#                [NO_INSTALL]
#   )
#
# The plugin library's name is constructed from the specified name, its
# specified plugin type (eg service, module, source), and (unless
# BASENAME_ONLY is specified) the package subdirectory path (replacing
# "/" with "_").
#
# Options:
#
# ALLOW_UNDERSCORES
#
#   Allow underscores in subdirectory names. Discouraged, as it creates
#   a possible ambiguity in the encoded plugin library name (art_test/XX
#   is indistinguishable from art/test/XX).
#
# BASENAME_ONLY
#
#    Omit the subdirectory path from the library name. Discouraged, as
#    it creates an ambiguity between modules with the same source
#    filename in different packages or different subdirectories within
#    the same package. The latter case is not possible however, because
#    CMake will throw an error because the two CMake targets will have
#    the same name and that is not permitted.
#
# NO_INSTALL
#
#    If specified, the plugin library will not be part of the installed
#    product (use for test modules, etc.).
#
# USE_PRODUCT_NAME
#
#   Passed through to basic_plugin
#
########################################################################

include(BasicPlugin)

macro (_sp_debug_message)
  string(TOUPPER ${CMAKE_BUILD_TYPE} BTYPE_UC)
  if (BTYPE_UC STREQUAL "DEBUG")
    message(STATUS "SIMPLE_PLUGIN: " ${ARGN})
  endif()
endmacro()

# simple plugin libraries
function(simple_plugin name type)
  set(simple_plugin_liblist)
  if(ART_FRAMEWORK_CORE)
    # using art as a product
    if("${type}" STREQUAL "service")
      list(INSERT simple_plugin_liblist 0 ${ART_FRAMEWORK_SERVICES_REGISTRY} ${FHICLCPP} ${CETLIB})
    elseif("${type}" STREQUAL "module" OR "${type}" STREQUAL "source")
      list(INSERT simple_plugin_liblist 0
	      ${ART_FRAMEWORK_CORE}
	      ${ART_FRAMEWORK_PRINCIPAL}
	      ${ART_PERSISTENCY_COMMON}
	      ${ART_PERSISTENCY_PROVENANCE}
	      ${ART_UTILITIES}
        ${FHICLCPP}
        ${CETLIB}
	      ${ROOT_CORE}
	      )
    endif()
  else()
    # this block is used when building art
    if("${type}" STREQUAL "service")
      list(INSERT simple_plugin_liblist 0 art_Framework_Services_Registry)
    elseif("${type}" STREQUAL "module" OR "${type}" STREQUAL "source")
      list(INSERT simple_plugin_liblist 0
        art_Framework_Core
        art_Framework_Principal
        art_Persistency_Provenance
        art_Utilities
        ${ROOT_CORE}
        )
    endif()
    if ("${type}" STREQUAL "source")
      list(INSERT simple_plugin_liblist 0
        art_Framework_IO_Sources
        )
    endif()
  endif()
  basic_plugin(${name} ${type} ${ARGN} ${simple_plugin_liblist})
endfunction(simple_plugin name type)
