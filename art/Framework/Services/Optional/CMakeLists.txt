set( art_Framework_Services_Optional_sources
  MyServiceInterface.cc
  TFileDirectory.cc
  detail/TH1AddDirectorySentry.cc
)

add_library(art_Framework_Services_Optional
  SHARED
  ${art_Framework_Services_Optional_sources}
)

target_link_libraries(art_Framework_Services_Optional
  ${MF_MESSAGELOGGER}
  ${CETLIB}
  ${ROOT_HIST}
  ${ROOT_CORE}
)

install ( TARGETS art_Framework_Services_Optional
  DESTINATION ${flavorqual_dir}/lib )

simple_plugin(RandomNumberGenerator "service"
  art_Framework_Principal
  art_Persistency_Common
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${SIGC}
  ${CLHEP}
  )

simple_plugin(SimpleInteraction "service" art_Framework_Services_Interfaces)
simple_plugin(SimpleMemoryCheck "service" art_Persistency_Provenance)
simple_plugin(TFileService "service" art_Framework_Services_Optional ${ROOT_RIO})
simple_plugin(Timing "service" art_Persistency_Provenance)
simple_plugin(Tracer "service" art_Persistency_Provenance)
simple_plugin(TrivialFileDelivery "service")
simple_plugin(TrivialFileTransfer "service")
simple_plugin(MyService "service" art_Framework_Services_Optional)

install_headers(SUBDIRS detail)
install_source()
