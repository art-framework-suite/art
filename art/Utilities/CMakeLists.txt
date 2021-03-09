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
    RegexMatch.cc
    ScheduleID.cc
    SharedResource.cc
    TaskDebugMacros.cc
    UnixSignalHandlers.cc
    ensureTable.cc
    parent_path.cc
    unique_filename.cc
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
)

cet_make_library(LIBRARY_NAME toolMaker INTERFACE
  EXPORT_SET PluginSupport
  SOURCE make_tool.h
  LIBRARIES INTERFACE
    art::Utilities
    canvas::canvas
    cetlib::cetlib
    fhiclcpp::fhiclcpp
)

cet_make_library(LIBRARY_NAME tool INTERFACE
  EXPORT_SET PluginTypes
  SOURCE ToolMacros.h ToolConfigTable.h
  LIBRARIES INTERFACE
    art_plugin_support::toolMaker
    art::Utilities
    canvas::canvas
    art_plugin_support::plugin_config_macros
    fhiclcpp::types
    fhiclcpp::fhiclcpp
    cetlib::cetlib
    art_plugin_support::support_macros
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
