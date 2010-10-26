# project name
project(artParameterSetTest)

set( cpp_files ParameterSetID_t.cpp
	       ParameterSetRecursing_t.cpp
	       Registry_t.cpp
	       TestFWCoreParameterSetDriver.cpp
	       UntrackedParameters_t.cpp )

# this list is not yet complete
link_libraries( artParameterSet
                artFrameworkPluginManager
		artPersistencyProvenance
		artUtilities
                ${MF_MESSAGELOGGER}  
                ${MF_MESSAGESERVICE}  
                ${MF_UTILITIES}  
                ${MF_EXTENSIONS}  
		${MATRIX}
                ${Boost_FILESYSTEM_LIBRARY}
                ${Boost_PYTHON_LIBRARY}
                ${Boost_REGEX_LIBRARY}
		${SIGC}
		${PYTHON} )

add_executable( ParameterSetID_t ParameterSetID_t.cpp )
add_executable( ParameterSetRecursing_t ParameterSetRecursing_t.cpp )
add_executable( Registry_t Registry_t.cpp )
add_executable( testParameterSet processbuilder_t.cppunit.cpp
                                 makeprocess_t.cppunit.cc
				 makepset_t.cppunit.cc
				 ps_t.cppunit.cc )
add_executable( TestFWCoreParameterSetDriver TestFWCoreParameterSetDriver.cpp )
#add_executable( UntrackedParameters_t UntrackedParameters_t.cpp )

add_test( ParameterSetID_t          ${EXECUTABLE_OUTPUT_PATH}/ParameterSetID_t )
add_test( ParameterSetRecursing_t   ${EXECUTABLE_OUTPUT_PATH}/ParameterSetRecursing_t )
add_test( Registry_t                ${EXECUTABLE_OUTPUT_PATH}/Registry_t )
add_test( testParameterSet          ${EXECUTABLE_OUTPUT_PATH}/testParameterSet )
add_test( FWCoreParameterSetDriver  ${EXECUTABLE_OUTPUT_PATH}/TestFWCoreParameterSetDriver )
