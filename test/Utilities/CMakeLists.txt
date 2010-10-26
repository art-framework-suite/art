# project name
project(artUtilitiesTest)


file( GLOB cpp_files *.cpp )

link_libraries(  artUtilities 
                 ${MF_MESSAGELOGGER}  
                 ${MF_MESSAGESERVICE}  
                 ${MF_UTILITIES}  
                 ${MF_EXTENSIONS}  
                 ${REFLEX}
		 ${CINT}
                 ${Boost_FILESYSTEM_LIBRARY}  
                 ${Boost_THREAD_LIBRARY}
		 ${Boost_REGEX_LIBRARY}
		 )

foreach(file ${cpp_files} )
  get_filename_component(ename ${file} NAME_WE )
  add_executable( ${ename}  ${file} )
  add_test( ${ename} ${EXECUTABLE_OUTPUT_PATH}/${ename} )
endforeach(file ${cpp_files} )
