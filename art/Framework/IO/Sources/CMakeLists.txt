set(art_Framework_IO_Sources_sources
  detail/FileServiceProxy.cc
)

art_make_library( LIBRARY_NAME art_Framework_IO_Sources
                  SOURCE ${art_Framework_IO_Sources_sources}
                  LIBRARIES art_Framework_Services_Interfaces
  art_Framework_Services_Registry
  art_Utilities
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
