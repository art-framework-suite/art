install(FILES GetFileFormatVersion.h
              GetReleaseVersion.h 
	DESTINATION include/FWCore/Version/interface )

>>><<<
# project name
project(FWCoreVersion)

# version number
set (FWCoreVersion_VERSION_MAJOR 1)
set (FWCoreVersion_VERSION_MINOR 0)

set( FWCoreVersion_sources GetFileFormatVersion.cc GetReleaseVersion.cc )

add_library(FWCoreVersion SHARED ${FWCoreVersion_sources} )

install ( TARGETS FWCoreVersion DESTINATION lib )
