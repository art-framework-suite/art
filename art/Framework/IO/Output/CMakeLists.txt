# project name
project(IOPoolOutput)

# version number
set (IOPoolOutput_VERSION_MAJOR 1)
set (IOPoolOutput_VERSION_MINOR 0)

set( IOPoolOutput_sources PoolOutputModule.cc
			  RootOutputFile.cc
			  RootOutputTree.cc )

link_libraries( IOPoolCommon
                DataFormatsCommon
		DataFormatsProvenance
		FWCoreServiceRegistry
		FWCoreParameterSet
		FWCoreMessageLogger
		FWCoreFramework
		FWCoreUtilities
                ${Boost_FILESYSTEM_LIBRARY}
                ${Boost_THREAD_LIBRARY}
		${REFLEX}
		${CORE} )

add_library(IOPoolOutput SHARED ${IOPoolOutput_sources} )

simple_plugin(PoolOutputModule_plugin)

install ( TARGETS IOPoolOutput DESTINATION lib )
