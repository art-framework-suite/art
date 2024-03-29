cet_make_library(SOURCE
    ActivityRegistry.cc
    ServiceRegistry.cc
    ServicesManager.cc
    detail/ServiceCacheEntry.cc
    detail/ensure_only_one_thread.cc
  LIBRARIES
  PUBLIC
    art::Persistency_Provenance
    art::Utilities
    canvas::canvas
    fhiclcpp::types
    fhiclcpp::fhiclcpp
    cetlib::cetlib
    cetlib_except::cetlib_except
  PRIVATE
    range-v3::range-v3
)

cet_make_library(LIBRARY_NAME serviceDeclaration INTERFACE
  EXPORT_SET ServicePluginType
  SOURCE ServiceDeclarationMacros.h
  LIBRARIES INTERFACE
    art::Framework_Services_Registry
    art::Utilities
    canvas::canvas
    fhiclcpp::headers
)

cet_make_library(LIBRARY_NAME systemService INTERFACE
  EXPORT_SET ServicePluginType NO_EXPORT
  SOURCE detail/system_service_macros.h
  LIBRARIES INTERFACE canvas::canvas)
# Not to be installed or collected.
cet_write_plugin_builder(systemService art::service Modules)

cet_make_library(LIBRARY_NAME serviceDefinition INTERFACE
  EXPORT_SET ServicePluginType
  SOURCE ServiceDefinitionMacros.h
  LIBRARIES INTERFACE
    canvas::canvas
    fhiclcpp::headers
    art_plugin_support::support_macros
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
