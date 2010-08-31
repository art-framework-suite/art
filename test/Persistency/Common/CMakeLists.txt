# project name
project(DataFormatsCommonTest)

file( GLOB cpp_files *.cpp )
set( cc_files testOwnVector.cc 
              testOneToOneAssociation.cc 
              testValueMap.cc 
              testOneToManyAssociation.cc 
              testAssociationVector.cc 
              testAssociationNew.cc 
              testValueMapNew.cc 
              testSortedCollection.cc 
              testRangeMap.cc 
              testIDVectorMap.cc 
              ref_t.cppunit.cc 
              DetSetRefVector_t.cppunit.cc 
              DetSetLazyVector_t.cppunit.cc 
              reftobase_t.cppunit.cc 
              reftobasevector_t.cppunit.cc 
              cloningptr_t.cppunit.cc 
              ptr_t.cppunit.cc 
              ptrvector_t.cppunit.cc 
              IntValues_reflex.cc )

link_libraries( DataFormatsCommon
                DataFormatsProvenance
                FWCoreUtilities
                FWCoreServiceRegistry
		${Boost_THREAD_LIBRARY}
		${Boost_PYTHON_LIBRARY}
		${MATRIX}
		${REFLEX} 
		${CORE} 
		${PYTHON} )


foreach(file ${cpp_files} )
  if( ${file} MATCHES testRunner.cpp )
  elseif( ${file} MATCHES ReflexTools_t.cpp )
  elseif( ${file} MATCHES DetSetNew_t.cpp )
  else()
    get_filename_component(ename ${file} NAME_WE )
    add_executable( ${ename}  ${file} )
    add_test( ${ename} ${EXECUTABLE_OUTPUT_PATH}/${ename} )
  endif()
endforeach(file ${cpp_files} )

add_executable( testDataFormatsCommon testRunner.cpp ${cc_files} )
add_test( testDataFormatsCommon ${EXECUTABLE_OUTPUT_PATH}/testDataFormatsCommon )

add_executable( ReflexTools_t ReflexTools_t.cpp )
target_link_libraries( ReflexTools_t DataFormatsWrappedStdDictionaries_dict_plugin
                                     DataFormatsWrappedStdDictionaries_map_plugin  )
add_test( ReflexTools_t ${EXECUTABLE_OUTPUT_PATH}/ReflexTools_t )
