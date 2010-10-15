# project name
project(artFrameworkIOCatalog)

set( artFrameworkIOCatalog_sources FileCatalog.cc
                                   InputFileCatalog.cc )

#link_libraries( artFrameworkServicesRegistry
#                artParameterSet
#                artUtilities
#                ${Boost_FILESYSTEM_LIBRARY}  )

add_library(artFrameworkIOCatalog SHARED ${artFrameworkIOCatalog_sources} )

install ( TARGETS artFrameworkIOCatalog DESTINATION ${flavorqual_dir}/lib )

install(FILES FileCatalog.h
              InputFileCatalog.h
              SiteLocalConfig.h
         DESTINATION  ${product}/${version}/include/art/Framework/IO/Catalog )
