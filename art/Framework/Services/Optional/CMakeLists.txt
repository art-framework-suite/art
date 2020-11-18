if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(MemoryTrackerSource MemoryTrackerLinux_service.cc)
else()
  set(MemoryTrackerSource MemoryTrackerDarwin_service.cc)
endif()

simple_plugin(RandomNumberGenerator "service"
  LIBRARIES PRIVATE
  art::art_Framework_Principal
  CLHEP::CLHEP
  messagefacility::MF_MessageLogger
)

basic_plugin(MemoryTracker "service"
  LIBRARIES PRIVATE
  art::art_Framework_Principal
  art::art_Framework_Services_Registry
  canvas::canvas
  SQLite::SQLite3
  messagefacility::MF_MessageLogger
  SOURCE ${MemoryTrackerSource})

simple_plugin(TimeTracker "service"
  LIBRARIES PRIVATE
  art::art_Framework_Principal
  TBB::tbb
  SQLite::SQLite3
  messagefacility::MF_MessageLogger)

simple_plugin(Tracer "service" LIBRARIES PRIVATE art_Framework_Principal messagefacility::MF_MessageLogger)
simple_plugin(TrivialFileDelivery "service")
simple_plugin(TrivialFileTransfer "service")

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
