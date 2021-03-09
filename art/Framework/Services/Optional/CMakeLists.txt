build_plugin(RandomNumberGenerator "service"
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

build_plugin(TimeTracker "service"
  LIBRARIES REG
    art::Framework_Principal
    art::Framework_Services_Registry
    art::Utilities
    messagefacility::MF_MessageLogger
    fhiclcpp::types
    cetlib::sqlite
    TBB::tbb
)

build_plugin(Tracer "service"
  LIBRARIES REG
    art::Framework_Principal
    art::Framework_Services_Registry
    art::Persistency_Provenance
    art::Utilities
    canvas::canvas
    messagefacility::MF_MessageLogger
    fhiclcpp::types
)

build_plugin(TrivialFileTransfer "service"
  BASE FileTransferService)

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

build_plugin(MemoryTracker "service"
  REG_SOURCE MemoryTracker${CMAKE_SYSTEM_NAME}_service.cc
  LIBRARIES PRIVATE
    ${mtracker_${CMAKE_SYSTEM_NAME}_libraries}
)

build_plugin(TrivialFileDelivery "service" BASE FileDeliveryService)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
