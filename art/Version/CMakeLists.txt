# project name
project(artVersion)

set( artVersion_sources GetFileFormatVersion.cc GetReleaseVersion.cc )

add_library(artVersion SHARED ${artVersion_sources} )

install ( TARGETS artVersion DESTINATION lib )
install(FILES GetFileFormatVersion.h
              GetReleaseVersion.h 
	DESTINATION include/art/Version )

