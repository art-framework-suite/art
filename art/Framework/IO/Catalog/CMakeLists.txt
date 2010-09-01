
install(FILES FileCatalog.h
              InputFileCatalog.h
	      SiteLocalConfig.h
	DESTINATION include/FWCore/Catalog/interface )
>>><<<
# project name
project(FWCoreCatalog)

# version number
set (FWCoreCatalog_VERSION_MAJOR 1)
set (FWCoreCatalog_VERSION_MINOR 0)

set( FWCoreCatalog_sources FileCatalog.cc InputFileCatalog.cc )

link_libraries( FWCoreServiceRegistry
                FWCoreParameterSet
		FWCoreUtilities
                ${Boost_FILESYSTEM_LIBRARY}  )

add_library(FWCoreCatalog SHARED ${FWCoreCatalog_sources} )

install ( TARGETS FWCoreCatalog DESTINATION lib )
