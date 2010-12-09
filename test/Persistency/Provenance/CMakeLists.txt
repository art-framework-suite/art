# project name
project(DataFormatsProvenanceTest)


file( GLOB cpp_files *.cpp )
file( GLOB cc_files *.cc )

link_libraries( DataFormatsProvenance
                art_Utilities
                art_Version
		${Boost_THREAD_LIBRARY}
		${REFLEX} ${CORE} )

foreach(file ${cpp_files} )
  if( ${file} MATCHES testRunner.cpp )
  else()
    get_filename_component(ename ${file} NAME_WE )
    add_executable( ${ename}  ${file} )
    add_test( ${ename} ${EXECUTABLE_OUTPUT_PATH}/${ename} )
  endif()
endforeach(file ${cpp_files} )

add_executable( testDataFormatsProvenance testRunner.cpp ${cc_files} )
add_test( testDataFormatsProvenance ${EXECUTABLE_OUTPUT_PATH}/testDataFormatsProvenance )
