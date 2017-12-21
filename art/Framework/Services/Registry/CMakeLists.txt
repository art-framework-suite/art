set(art_Framework_Services_Registry_sources
  ServiceRegistry.cc
  ServicesManager.cc
  detail/ServiceCacheEntry.cc)

art_make_library(LIBRARY_NAME art_Framework_Services_Registry
  SOURCE ${art_Framework_Services_Registry_sources}
  LIBRARIES ${TBB}
  ${Boost_THREAD_LIBRARY}
  art_Utilities
  canvas)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
