include(art::FileDeliveryService)
include(art::FileTransferService)

cet_build_plugin(RandomNumberGenerator art::service
  LIBRARIES PUBLIC
    art::Framework_Services_Registry
    art::Persistency_Provenance
    art::Utilities
    canvas::canvas
    fhiclcpp::types
    CLHEP::Random
  PRIVATE
    art::Framework_Principal
    messagefacility::MF_MessageLogger
    cetlib::container_algorithms
    hep_concurrency::macros
    cetlib_except::cetlib_except
)

cet_build_plugin(TimeTracker art::service
  LIBRARIES REG
    art::Framework_Principal
    art::Framework_Services_Registry
    art::Utilities
    messagefacility::MF_MessageLogger
    fhiclcpp::types
    cetlib::sqlite
    TBB::tbb
)

cet_build_plugin(Tracer art::service
  LIBRARIES REG
    art::Framework_Principal
    art::Framework_Services_Registry
    art::Persistency_Provenance
    art::Utilities
    canvas::canvas
    messagefacility::MF_MessageLogger
    fhiclcpp::types
)

cet_build_plugin(TrivialFileTransfer art::FileTransferService)

set(mtracker_Linux_libraries
  PRIVATE
    art::Framework_Services_Registry
    art::Framework_Principal
    art::Persistency_Provenance
    art::Utilities
    canvas::canvas
    messagefacility::MF_MessageLogger
    fhiclcpp::types
    cetlib::container_algorithms
    cetlib::sqlite
    cetlib::cetlib
)

set(mtracker_Darwin_libraries)

cet_build_plugin(MemoryTracker art::service
  REG_SOURCE MemoryTracker${CMAKE_SYSTEM_NAME}_service.cc
  LIBRARIES PRIVATE
    ${mtracker_${CMAKE_SYSTEM_NAME}_libraries}
)

cet_build_plugin(TrivialFileDelivery art::FileDeliveryService)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
