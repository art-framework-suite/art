set( art_Framework_Services_Optional_sources
  TFileDirectory.cc
  detail/TH1AddDirectorySentry.cc
)

art_make_library( LIBRARY_NAME art_Framework_Services_Optional
                  SOURCE ${art_Framework_Services_Optional_sources}
                  LIBRARIES ${CETLIB}
  ${ROOT_HIST}
  ${ROOT_CORE}
)

simple_plugin(RandomNumberGenerator "service"
  art_Framework_Principal
  art_Persistency_Common
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${SIGC}
  ${CLHEP}
  )

simple_plugin(SimpleInteraction "service" art_Framework_Services_UserInteraction)
simple_plugin(SimpleMemoryCheck "service" art_Persistency_Provenance)
simple_plugin(TFileService "service" art_Framework_Services_Optional ${ROOT_RIO})
simple_plugin(Timing "service" art_Persistency_Provenance)
simple_plugin(Tracer "service" art_Persistency_Provenance)
simple_plugin(TrivialFileDelivery "service")
simple_plugin(TrivialFileTransfer "service")

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
