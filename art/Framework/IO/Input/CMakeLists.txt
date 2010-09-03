# project name
project(artFrameworkIOInput)

set( artFrameworkIOInput_sources BranchMapperWithReader.cc
			 DuplicateChecker.cc
			 Module.cc
			 PoolSource.cc
			 ProvenanceAdaptor.cc
			 RootDelayedReader.cc
			 RootFile.cc
			 RootInputFileSequence.cc
			 RootTree.cc )


#link_libraries( artFrameworkIOCommon
#                artPersistencyCommon
#		artPersistencyProvenance
#		artFrameworkIOSources
#		artFrameworkIOCatalog
#		artFrameworkServicesRegistry
#		artParameterSet
#		artMessageLogger
#		artFrameworkCore
#		artUtilities
#                ${Boost_FILESYSTEM_LIBRARY}
#                ${Boost_THREAD_LIBRARY}
#		${CLHEP}
#		${REFLEX}
#		${CORE} )

add_library(artFrameworkIOInput SHARED ${artFrameworkIOInput_sources} )

simple_plugin(PoolRASource_plugin)
simple_plugin(PoolSource_plugin)

install ( TARGETS artFrameworkIOInput DESTINATION lib )
