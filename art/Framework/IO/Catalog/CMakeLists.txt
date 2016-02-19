set(art_Framework_IO_Catalog_sources
  FileCatalog.cc
  InputFileCatalog.cc
  )

art_make_library( LIBRARY_NAME art_Framework_IO_Catalog
                  SOURCE ${art_Framework_IO_Catalog_sources}
                  LIBRARIES art_Framework_Services_Optional_TrivialFileDelivery_service
  art_Framework_Services_Optional_TrivialFileTransfer_service
  art_Utilities canvas_Utilities
  ${Boost_FILESYSTEM_LIBRARY}
)

install_headers()
install_source()
