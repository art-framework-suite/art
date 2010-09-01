# project name
project(FWCoreRootAutoLibraryLoaderTest)

set( cpp_files stdnamespaceadder_t.cppunit.cpp )


link_libraries( FWCoreRootAutoLibraryLoader
                FWCorePluginManager
                DataFormatsProvenance
                ${Boost_FILESYSTEM_LIBRARY}
                ${Boost_REGEX_LIBRARY}
                ${CORE}
                ${REFLEX}
                ${CINT} dl )

add_executable( testFWCoreRootAutoLibraryLoader stdnamespaceadder_t.cppunit.cpp )

add_test( FWCoreRootAutoLibraryLoader ${EXECUTABLE_OUTPUT_PATH}/testFWCoreRootAutoLibraryLoader)

