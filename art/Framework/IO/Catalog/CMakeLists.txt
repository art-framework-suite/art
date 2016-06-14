set(art_Framework_IO_Catalog_sources
  FileCatalog.cc
  InputFileCatalog.cc
  )

#link_libraries(
#  art_Utilities
#  ${Boost_FILESYSTEM_LIBRARY}
#  )

art_make_library( LIBRARY_NAME art_Framework_IO_Catalog
                  SOURCE ${art_Framework_IO_Catalog_sources}
                  LIBRARIES art_Framework_Services_Optional_TrivialFileDelivery_service
  art_Framework_Services_Optional_TrivialFileTransfer_service
  art_Utilities
  ${Boost_FILESYSTEM_LIBRARY}
)

install_headers()
#install(FILES FileCatalog.h
#              InputFileCatalog.h
#         DESTINATION  ${product}/${version}/include/art/Framework/IO/Catalog )

install_source()
