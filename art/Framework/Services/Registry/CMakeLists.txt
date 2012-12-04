# project name
project(art_Framework_Services_Registry)

set(art_Framework_Services_Registry_sources
  ServiceRegistry.cc
  ServicesManager.cc
  detail/ServiceCacheEntry.cc
  )

art_make_library( LIBRARY_NAME art_Framework_Services_Registry
                  SOURCE ${art_Framework_Services_Registry_sources}
                  LIBRARIES art_Utilities
  ${SIGC}
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
