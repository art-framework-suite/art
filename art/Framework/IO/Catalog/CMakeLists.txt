set(art_Framework_IO_Catalog_sources
  FileCatalog.cc
  InputFileCatalog.cc
  )

link_libraries(
  art_Utilities
  ${Boost_FILESYSTEM_LIBRARY}
  )

add_library(art_Framework_IO_Catalog SHARED ${art_Framework_IO_Catalog_sources} )
target_link_libraries(art_Framework_IO_Catalog
  art_Framework_Services_Optional_TrivialFileDelivery_service
  art_Framework_Services_Optional_TrivialFileTransfer_service
)

install ( TARGETS art_Framework_IO_Catalog DESTINATION ${flavorqual_dir}/lib )

install(FILES FileCatalog.h
              InputFileCatalog.h
         DESTINATION  ${product}/${version}/include/art/Framework/IO/Catalog )

install_source()
