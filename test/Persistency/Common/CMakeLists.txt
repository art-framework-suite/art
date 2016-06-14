# project name
project(DataFormatsCommonTest)

file( GLOB cpp_files *.cpp )
set( cc_files ptr_t.cppunit.cc
              IntValues_reflex.cc )

link_libraries( art_Persistency_Common
                art_Persistency_Provenance
                art_Utilities
                art_Framework_Services_Registry
		${Boost_THREAD_LIBRARY}
		${Boost_PYTHON_LIBRARY}
		${ROOT_MATRIX}
		${ROOT_REFLEX}
		${ROOT_CORE}
		${PYTHON} )


foreach(file ${cpp_files} )
  if( ${file} MATCHES testRunner.cpp )
  elseif( ${file} MATCHES ReflexTools_t.cpp )
  else()
    get_filename_component(ename ${file} NAME_WE )
    add_executable( ${ename}  ${file} )
    add_test( ${ename} ${EXECUTABLE_OUTPUT_PATH}/${ename} )
  endif()
endforeach(file ${cpp_files} )

add_executable( testDataFormatsCommon testRunner.cpp ${cc_files} )
add_test( testDataFormatsCommon ${EXECUTABLE_OUTPUT_PATH}/testDataFormatsCommon )

add_executable( ReflexTools_t ReflexTools_t.cpp )
target_link_libraries( ReflexTools_t DataFormatsWrappedStdDictionaries_dict
                                     DataFormatsWrappedStdDictionaries_map  )
add_test( ReflexTools_t ${EXECUTABLE_OUTPUT_PATH}/ReflexTools_t )
