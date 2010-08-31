# project name
project(FWCoreUtilitiesTest)


file( GLOB cpp_files *.cpp )

link_libraries(  FWCoreUtilities ${Boost_FILESYSTEM_LIBRARY}  )

foreach(file ${cpp_files} )
  get_filename_component(ename ${file} NAME_WE )
  add_executable( ${ename}  ${file} )
  add_test( ${ename} ${EXECUTABLE_OUTPUT_PATH}/${ename} )
endforeach(file ${cpp_files} )
