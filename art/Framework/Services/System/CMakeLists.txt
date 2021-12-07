########################################################################
# Important Note
#
#   System "services" are not in fact true plugins as they need no
#   plugin registration library.
########################################################################

########################################################################
# These services are not actually true "system" services, as they use
# the standard service definition macro.
cet_build_plugin(DatabaseConnection art::service
  LIBRARIES CONDITIONAL cetlib::sqlite)

cet_build_plugin(FileCatalogMetadata art::service LIBRARIES
  PUBLIC
    canvas::canvas
    fhiclcpp::types
    cetlib::container_algorithms
    cetlib::cetlib
  PRIVATE Boost::json)
########################################################################

# Learn about building system services.
include(systemService)

# FloatingPoint is a true "system" service (constructed explicitly by
# art::EventProcessor), but has a registration module due to the use of
# CET_PROVIDE_FILE_PATH() and FHICL_PROVIDE_ALLOWED_CONFIGURATION().
cet_build_plugin(FloatingPointControl systemService
  IMPL_SOURCE FloatingPointControl.cc detail/fpControl.cc
  LIBRARIES
  PUBLIC art::Framework_Services_Registry fhiclcpp::types
  PRIVATE messagefacility::MF_MessageLogger)

# TriggerNamesService is a "true" system service, which means it's not
# *actually* a plugin since it has no registration code.
cet_make_library(LIBRARY_NAME art_Framework_Services_System_TriggerNamesService_service
  SOURCE TriggerNamesService_service.cc
  LIBRARIES PUBLIC art_plugin_types::systemService
    fhiclcpp::fhiclcpp
    art::Persistency_Provenance
    art::Utilities
    art::Framework_Principal
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
