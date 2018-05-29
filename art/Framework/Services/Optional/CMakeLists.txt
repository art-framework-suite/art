find_package(ROOT REQUIRED COMPONENTS Hist Matrix MathCore Core RIO Thread Tree)

set(art_Framework_Services_Optional_sources
  TFileDirectory.cc
  detail/RootDirectorySentry.cc)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(MemoryTrackerSource MemoryTrackerLinux_service.cc)
else()
  set(MemoryTrackerSource MemoryTrackerDarwin_service.cc)
endif()

art_make_library(SOURCE ${art_Framework_Services_Optional_sources}
  LIBRARIES cetlib
  ${ROOT_Hist_LIBRARY}
  ${ROOT_Matrix_LIBRARY}
  ${ROOT_MathCore}
  ${ROOT_Core_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_REGEX_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  art_Framework_Services_Registry
  art_Utilities
  canvas
  cetlib)

simple_plugin(RandomNumberGenerator "service"
  art_Framework_Principal
  art_Persistency_Common
  MF_MessageLogger
  cetlib
  CLHEP::Random)

simple_plugin(MemoryAdjuster "service"
  art_Framework_Services_Optional)

basic_plugin(MemoryTracker "service"
  art_Framework_Services_Optional
  canvas
  cetlib
  ${SQLITE3}
  SOURCE ${MemoryTrackerSource})

simple_plugin(TFileService "service"
  art_Framework_Services_System_TriggerNamesService_service
  art_Framework_Services_Optional
  art_Framework_IO
  art_Framework_IO_detail
  art_Framework_Principal
  MF_MessageLogger
  ${ROOT_RIO_LIBRARY}
  ${ROOT_Thread_LIBRARY}
  ${ROOT_Tree_LIBRARY})

simple_plugin(TimeTracker "service"
  art_Framework_Principal
  canvas
  cetlib
  ${TBB}
  ${RT}
  ${SQLITE3})

simple_plugin(Tracer "service" canvas)
simple_plugin(TrivialFileDelivery "service")
simple_plugin(TrivialFileTransfer "service")

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
