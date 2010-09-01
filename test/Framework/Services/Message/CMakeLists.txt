# project name
project(FWCoreMessageServicesTest)

file( GLOB cpp_files *.cpp )
file( GLOB cc_files *.cc )

link_libraries( DataFormatsProvenance
		FWCoreServiceRegistry
		FWCoreMessageLogger
		FWCoreParameterSet
		FWCoreUtilities
		${MATRIX}
                ${Boost_THREAD_LIBRARY} )

foreach(file ${cc_files} )
  get_filename_component(ename ${file} NAME_WE )
  add_library(${ename} SHARED ${file} )
endforeach(file ${cpp_files} )

foreach(file ${cpp_files} )
  get_filename_component(ename ${file} NAME_WE )
  add_executable( ${ename}  ${file} )
  add_test( ${ename} ${EXECUTABLE_OUTPUT_PATH}/${ename} )
endforeach(file ${cpp_files} )
