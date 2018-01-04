set(art_Utilities_sources
  ensureTable.cc
  ExceptionMessages.cc
  LinuxProcMgr.cc
  MallocOpts.cc
  parent_path.cc
  PluginSuffixes.cc
  RegexMatch.cc
  RootHandlers.cc
  unique_filename.cc
  UnixSignalHandlers.cc)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  list(APPEND art_Utilities_sources LinuxProcMgr.cc)
endif()

art_make_library(SOURCE ${art_Utilities_sources}
  LIBRARIES canvas
  MF_MessageLogger
  fhiclcpp
  cetlib
  cetlib_except
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  pthread
  ${CMAKE_DL_LIBS}
  )

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
