set(art_Framework_IO_Root_Services_sources
  TFileDirectory.cc
  detail/RootDirectorySentry.cc)

art_make_library(SOURCE ${art_Framework_IO_Root_Services_sources}
  LIBRARIES cetlib
  ${ROOT_HIST}
  ${ROOT_MATRIX}
  ${ROOT_MATHCORE}
  ${ROOT_CORE}
  ${ROOT_CINT}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_REGEX_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  art_Framework_IO_Root
  art_Framework_Services_Registry
  art_Utilities
  canvas
  cetlib)

simple_plugin(TFileService "service"
  art_Framework_Services_System_TriggerNamesService_service
  art_Framework_IO_Root_Services
  art_Framework_IO
  art_Framework_IO_detail
  art_Framework_Principal
  ${ROOT_RIO}
  ${ROOT_THREAD}
  ${ROOT_TREE})

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
