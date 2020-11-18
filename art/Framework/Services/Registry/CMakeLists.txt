art_make_library(LIBRARY_NAME art_Framework_Services_Registry
  SOURCE
    ServiceRegistry.cc
    ServicesManager.cc
    detail/ServiceCacheEntry.cc
    detail/ensure_only_one_thread.cc
  LIBRARIES
    ${TBB}
    art::art_Persistency_Provenance
    art::art_Utilities
  PRIVATE
    canvas::canvas)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
