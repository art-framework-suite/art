simple_plugin(RandomNumberGenerator "service"
  LIBRARIES PRIVATE
    art::art_Framework_Principal
    CLHEP::CLHEP
    messagefacility::MF_MessageLogger
)

basic_plugin(MemoryTracker "service"
  SOURCE
    $<IF:$<PLATFORM_ID:Darwin>,MemoryTrackerDarwin_service.cc,MemoryTrackerLinux_service.cc>
  LIBRARIES PRIVATE
    art::art_Framework_Principal
    art::art_Framework_Services_Registry
    canvas::canvas
    SQLite::SQLite3
    messagefacility::MF_MessageLogger
)

simple_plugin(TimeTracker "service"
  LIBRARIES PRIVATE
    art::art_Framework_Principal
    TBB::tbb
    SQLite::SQLite3
    messagefacility::MF_MessageLogger
)

simple_plugin(Tracer "service" LIBRARIES PRIVATE art::art_Framework_Principal messagefacility::MF_MessageLogger)
simple_plugin(TrivialFileDelivery "service")
simple_plugin(TrivialFileTransfer "service")

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
