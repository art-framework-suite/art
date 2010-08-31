# project name
project(IOPoolInput)

# version number
set (IOPoolInput_VERSION_MAJOR 1)
set (IOPoolInput_VERSION_MINOR 0)

set( IOPoolInput_sources BranchMapperWithReader.cc
			 DuplicateChecker.cc
			 Module.cc
			 PoolSource.cc
			 ProvenanceAdaptor.cc
			 RootDelayedReader.cc
			 RootFile.cc
			 RootInputFileSequence.cc
			 RootTree.cc )


link_libraries( IOPoolCommon
                DataFormatsCommon
		DataFormatsProvenance
		FWCoreSources
		FWCoreCatalog
		FWCoreServiceRegistry
		FWCoreParameterSet
		FWCoreMessageLogger
		FWCoreFramework
		FWCoreUtilities
                ${Boost_FILESYSTEM_LIBRARY}
                ${Boost_THREAD_LIBRARY}
		${CLHEP}
		${REFLEX}
		${CORE} )

add_library(IOPoolInput SHARED ${IOPoolInput_sources} )

simple_plugin(PoolRASource_plugin)
simple_plugin(PoolSource_plugin)

install ( TARGETS IOPoolInput DESTINATION lib )
