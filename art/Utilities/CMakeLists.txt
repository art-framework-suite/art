cet_make_library(LIBRARY_NAME ParameterSetHelpers_CLHEP
  INTERFACE
  SOURCE ParameterSetHelpers/CLHEP_ps.h
  LIBRARIES INTERFACE CLHEP::Vector
)

cet_make_library(
  SOURCE
    $<$<PLATFORM_ID:Linux>:LinuxProcMgr.cc>
    ExceptionMessages.cc
    GlobalTaskGroup.cc
    Globals.cc
    MallocOpts.cc
    PluginSuffixes.cc
    ScheduleID.cc
    SharedResource.cc
    TaskDebugMacros.cc
    UnixSignalHandlers.cc
    ensureTable.cc
    parent_path.cc
    unique_filename.cc
    detail/remove_whitespace.cc
  LIBRARIES
  PUBLIC
    canvas::canvas
    fhiclcpp::fhiclcpp
    cetlib::cetlib
    hep_concurrency::hep_concurrency
    cetlib_except::cetlib_except
    TBB::tbb
  PRIVATE
    messagefacility::MF_MessageLogger
    hep_concurrency::macros
    Boost::filesystem
    range-v3::range-v3
)

cet_register_export_set(SET_NAME PluginSupport NAMESPACE art_plugin_support)

cet_make_library(LIBRARY_NAME toolMaker INTERFACE
  EXPORT_SET PluginSupport
  SOURCE make_tool.h detail/tool_type.h
  LIBRARIES INTERFACE
    art::Utilities
    canvas::canvas
    cetlib::cetlib
    fhiclcpp::fhiclcpp
)

cet_register_export_set(SET_NAME ToolType NAMESPACE art_plugin_types)

cet_make_library(LIBRARY_NAME tool INTERFACE
  EXPORT_SET ToolType
  SOURCE ToolMacros.h
  LIBRARIES INTERFACE
  art_plugin_support::plugin_config_macros
  fhiclcpp::types
  fhiclcpp::fhiclcpp
  cetlib::compiler_macros
  art_plugin_support::support_macros
)

cet_make_library(LIBRARY_NAME tool_interface INTERFACE
  EXPORT_SET ToolType
  SOURCE ToolConfigTable.h
  LIBRARIES INTERFACE
  fhiclcpp::types
  fhiclcpp::fhiclcpp
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
