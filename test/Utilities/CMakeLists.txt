# project name
project(art_Utilities_Test)


file( GLOB cpp_files *.cpp )

link_libraries(  art_Utilities
                 ${MF_MESSAGELOGGER}
                 ${MF_MESSAGESERVICE}
                 ${MF_UTILITIES}
                 ${MF_EXTENSIONS}
                 )

foreach(file ${cpp_files} )
  get_filename_component(ename ${file} NAME_WE )
  add_executable( ${ename}  ${file} )
  add_test( ${ename} ${EXECUTABLE_OUTPUT_PATH}/${ename} )
endforeach(file ${cpp_files} )
