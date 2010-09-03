# project name
project(artFrameworkIOSources)

set( artFrameworkIOSources_sources EDInputSource.cc
			   ExternalInputSource.cc
			   RawInputSource.cc
			   VectorInputSource.cc
			   VectorInputSourceFactory.cc )

#link_libraries( ${Boost_FILESYSTEM_LIBRARY}  )

add_library(artFrameworkIOSources SHARED ${artFrameworkIOSources_sources} )

install ( TARGETS artFrameworkIOSources DESTINATION lib )
install(FILES EDInputSource.h
	      ExternalInputSource.h
	      RawInputSource.h
	      VectorInputSourceFactory.h
	      VectorInputSource.h
	      VectorInputSourceMacros.h
	DESTINATION include/art/Framework/IO/Sources )
