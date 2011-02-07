# project name
project(art_Framework_IO_Catalog)

set( art_Framework_IO_Catalog_sources FileCatalog.cc
                                   InputFileCatalog.cc )

#link_libraries( art_Framework_Services_Registry
#                art_ParameterSet
#                art_Utilities
#                ${Boost_FILESYSTEM_LIBRARY}  )

add_library(art_Framework_IO_Catalog SHARED ${art_Framework_IO_Catalog_sources} )

install ( TARGETS art_Framework_IO_Catalog DESTINATION ${flavorqual_dir}/lib )

install(FILES FileCatalog.h
              InputFileCatalog.h
         DESTINATION  ${product}/${version}/include/art/Framework/IO/Catalog )
