set( art_Framework_Services_Optional_sources
  TFileDirectory.cc
  detail/TH1AddDirectorySentry.cc
)

art_make_library(# LIBRARY_NAME art_Framework_Services_Optional
                  SOURCE ${art_Framework_Services_Optional_sources}
                  LIBRARIES ${CETLIB}
  ${ROOT_HIST}
  ${ROOT_MATRIX}
  ${ROOT_MATHCORE}
  ${ROOT_CORE}
  ${ROOT_CINT}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_REGEX_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)

simple_plugin(RandomNumberGenerator "service"
  art_Framework_Principal
  art_Persistency_Common
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  ${CETLIB}
  ${CLHEP}
  )

simple_plugin(SimpleInteraction "service" art_Framework_Services_UserInteraction)
simple_plugin(SimpleMemoryCheck "service" art_Persistency_Provenance)
simple_plugin(TFileService "service"
  art_Framework_Services_System_TriggerNamesService_service
  art_Framework_Services_Optional
  art_Framework_IO
  art_Framework_Principal
  ${ROOT_RIO}
  ${ROOT_THREAD})
simple_plugin(Timing "service" art_Persistency_Provenance)
simple_plugin(Tracer "service" art_Persistency_Provenance)
simple_plugin(TrivialFileDelivery "service")
simple_plugin(TrivialFileTransfer "service")

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
