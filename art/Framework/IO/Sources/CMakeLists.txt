install(FILES EDInputSource.h
	      ExternalInputSource.h
	      RawInputSource.h
	      VectorInputSourceFactory.h
	      VectorInputSource.h
	      VectorInputSourceMacros.h
	DESTINATION include/FWCore/Sources/interface )
>>><<<
# project name
project(FWCoreSources)

# version number
set (FWCoreSources_VERSION_MAJOR 1)
set (FWCoreSources_VERSION_MINOR 0)

set( FWCoreSources_sources EDInputSource.cc
			   ExternalInputSource.cc
			   RawInputSource.cc
			   SConscript
			   VectorInputSource.cc
			   VectorInputSourceFactory.cc )

link_libraries( ${Boost_FILESYSTEM_LIBRARY}  )

add_library(FWCoreSources SHARED ${FWCoreSources_sources} )

install ( TARGETS FWCoreSources DESTINATION lib )
