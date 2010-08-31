
install(FILES ClassFiller.h  
              RefStreamer.h  
	      RootServiceChecker.h  
	      TransientStreamer.h
	DESTINATION include/IOPool/Common/interface )
>>><<<
# project name
project(IOPoolCommon)

# version number
set (IOPoolCommon_VERSION_MAJOR 1)
set (IOPoolCommon_VERSION_MINOR 0)

set( IOPoolCommon_sources RootServiceChecker.cc )

link_libraries( FWCoreUtilities
                FWCoreServiceRegistry )

add_library(IOPoolCommon SHARED ${IOPoolCommon_sources} )

install ( TARGETS IOPoolCommon DESTINATION lib )
