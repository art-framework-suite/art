if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(MemoryTrackerSource MemoryTrackerLinux_service.cc)
else()
  set(MemoryTrackerSource MemoryTrackerDarwin_service.cc)
endif()

simple_plugin(RandomNumberGenerator "service"
  art_Framework_Principal
  art_Persistency_Common
  CLHEP::Random)

basic_plugin(MemoryTracker "service"
  art_Framework_Principal
  canvas
  cetlib
  ${SQLITE3}
  SOURCE ${MemoryTrackerSource})

simple_plugin(TimeTracker "service"
  art_Framework_Principal
  TBB::tbb
  ${SQLITE3})

simple_plugin(Tracer "service" art_Framework_Principal)
simple_plugin(TrivialFileDelivery "service")
simple_plugin(TrivialFileTransfer "service")

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
