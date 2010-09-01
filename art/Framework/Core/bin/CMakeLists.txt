# project name
project(FWCoreFW)

# version number
set (FWCoreFW_VERSION_MAJOR 1)
set (FWCoreFW_VERSION_MINOR 0)

set( FWCoreFW_sources muteapp.cc )

link_libraries( FWCoreFW
                DataFormatsProvenance
                DataFormatsCommon
		FWCoreMessageLogger
		FWCoreServices
		FWCoreFramework
		FWCoreParameterSet
		FWCorePluginManager
		FWCoreServiceRegistry
		FWCoreUtilities
		FWCoreVersion
                ${Boost_FILESYSTEM_LIBRARY}
                ${Boost_PROGRAM_OPTIONS_LIBRARY}
                ${SIGC}
		${CORE}
		${REFLEX}
		${MATRIX} )

add_library(FWCoreFW SHARED ${FWCoreFW_sources} )

add_executable( mute fw.cpp )
add_executable( fw   fw.cpp   )

install ( TARGETS FWCoreFW DESTINATION lib )
install ( TARGETS fw   DESTINATION bin )
install ( TARGETS mute DESTINATION bin )
