set( art_Framework_Services_Optional_sources
  TFileDirectory.cc
  detail/TH1AddDirectorySentry.cc
)

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" ) 
  list( APPEND 
    art_Framework_Services_Optional_sources 
    MemoryTrackerLinux.cc
    SimpleMemoryCheckLinux.cc
    detail/LinuxProcMgr.cc
    )
endif()

art_make_library( SOURCE ${art_Framework_Services_Optional_sources}
  LIBRARIES cetlib
  ${ROOT_HIST}
  ${ROOT_MATRIX}
  ${ROOT_MATHCORE}
  ${ROOT_CORE}
  ${ROOT_CINT}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_REGEX_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  art_Framework_Services_Registry
  art_Persistency_Provenance
  art_Utilities
  art_Ntuple
)

simple_plugin(RandomNumberGenerator "service"
  art_Framework_Principal
  art_Persistency_Common
  MF_MessageLogger
  MF_Utilities
  cetlib
  ${CLHEP}
  )

simple_plugin(SimpleInteraction "service" art_Framework_Services_UserInteraction)

simple_plugin(SimpleMemoryCheck "service"
  art_Persistency_Provenance
  art_Framework_Services_Optional
  )

simple_plugin(MemoryAdjuster "service"
  art_Framework_Services_Optional
  )

simple_plugin(MemoryTracker "service"
  art_Framework_Services_Optional
  art_Ntuple
  art_Persistency_Provenance
  )

simple_plugin(TFileService "service"
  art_Framework_Services_System_TriggerNamesService_service
  art_Framework_Services_Optional
  art_Framework_IO
  art_Framework_Principal
  ${ROOT_RIO}
  ${ROOT_THREAD})

simple_plugin(Timing "service" art_Persistency_Provenance)

simple_plugin(TimeTracker "service"
  art_Ntuple
  art_Persistency_Provenance
  ${TBB}
  -lrt
  )

simple_plugin(Tracer "service" art_Persistency_Provenance)

simple_plugin(TrivialFileDelivery "service")

simple_plugin(TrivialFileTransfer "service")

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
