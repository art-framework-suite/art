# Declare the install path from the cmake command line
# using either a relative or full path
# Use setup_fw_development to define the build and code directories, then:
#   cd $ART_WORKDIR
#   cmake -DCMAKE_INSTALL_PREFIX=../install $ART_CODE (choose your install path)
#   make
#   make test
#   make install
# If you change any of the Cmake* files in $ART_CODE, 
# you need to run the cmake step again

# use cmake 2.6 or later
cmake_minimum_required (VERSION 2.6)
# project name
project(art)

# because we want to move these libraries about, 
# do not embed full path in shared libraries or executables
set (CMAKE_SKIP_RPATH)
# get verbose output so we can debug cmake
#set (CMAKE_VERBOSE_MAKEFILE TRUE)

ENABLE_TESTING()

# require ART_EXTERNALS
set( ART_EXTERNALS $ENV{ART_EXTERNALS} )
if ( NOT ART_EXTERNALS )
  message(FATAL_ERROR "Can't locate ART_EXTERNALS, required for building framework")
endif ()

# require ART_HOME
set( ART_HOME $ENV{ART_HOME} )
if ( NOT ART_HOME )
  message(FATAL_ERROR "Can't locate ART_HOME, required for building framework")
endif ()

# include search path
include_directories ("${PROJECT_BINARY_DIR}")
include_directories ("${PROJECT_SOURCE_DIR}")
# notice use of ENV to invoke an external variable
include_directories ( $ENV{CPPUNIT_DIR}/include )
include_directories ( $ENV{CLHEP_DIR}/include )
include_directories ( $ENV{ROOT_DIR}/include )
include_directories ( $ENV{ROOT_DIR}/cintex/inc )
include_directories ( $ENV{BOOST_INC} )
include_directories ( $ENV{LIBSIGCPP_DIR}/include/sigc++-2.0 )
include_directories ( $ENV{LIBSIGCPP_DIR}/lib/sigc++-2.0/include )

# define ROOT libraries
find_library( REFLEX NAMES Reflex PATHS $ENV{ROOTSYS}/lib )
find_library( CINT   NAMES Cint   PATHS $ENV{ROOTSYS}/lib )
find_library( CINTEX NAMES Cintex PATHS $ENV{ROOTSYS}/lib )
find_library( CORE   NAMES Core   PATHS $ENV{ROOTSYS}/lib )
find_library( HIST   NAMES Hist   PATHS $ENV{ROOTSYS}/lib )
find_library( TREE   NAMES Tree   PATHS $ENV{ROOTSYS}/lib )
find_library( GRAF   NAMES Graf   PATHS $ENV{ROOTSYS}/lib )
find_library( RIO    NAMES RIO    PATHS $ENV{ROOTSYS}/lib )
find_library( NET    NAMES Net    PATHS $ENV{ROOTSYS}/lib )
find_library( MATRIX NAMES Matrix PATHS $ENV{ROOTSYS}/lib )
# define genreflex executable
find_program( GENREFLEX NAMES genreflex PATHS $ENV{ROOTSYS}/bin )

# define CLHEP
find_library( CLHEP  NAMES CLHEP   PATHS $ENV{CLHEP_DIR}/lib )

# have to make sure cmake doesn't select the system libraries
# define libsigc 
find_library( SIGC    NAMES sigc-2.0  PATHS $ENV{LIBSIGCPP_DIR}/lib NO_DEFAULT_PATH )
# define cppunit
find_library( CPPUNIT NAMES cppunit   PATHS $ENV{CPPUNIT_DIR}/lib   NO_DEFAULT_PATH )

# define the boost environment so we don't get system libraries
set(BOOST_ROOT $ENV{BOOST_DIR} )
set(BOOST_INCLUDEDIR $ENV{BOOST_INC} )
set(BOOST_LIBRARYDIR $ENV{BOOST_LIB} )
set(Boost_USE_MULTITHREADED ON)
# search for Boost version 1.34 libraries
find_package( Boost 1.34 COMPONENTS filesystem 
                                    regex 
				    thread 
                                    program_options )


# define common flags
set( CMAKE_CXX_LINK_FLAGS "${CPPUNIT} -luuid -ldl" )

#build all libraries in a single directory to enable testing
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
#build all executables in a single directory
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# define some variables for genreflex
set( GENREFLEX_FLAGS --deep 
                     --fail_on_warnings 
		     --capabilities=classes_ids.cc 
                     -DCMS_DICT_IMPL 
		     -D_REENTRANT 
		     -DGNU_SOURCE 
		     -DGNU_GCC 
		     -DPROJECT_NAME="CMSSW" 
		     -DPROJECT_VERSION="CMSSW_3_0_0_pre2" )
set( GENREFLEX_INCLUDES -I $ENV{CPPUNIT_DIR}/include
			-I $ENV{CLHEP_DIR}/include
			-I $ENV{ROOT_DIR}/include
			-I $ENV{ROOT_DIR}/cintex/inc
			-I $ENV{BOOST_INC}
			-I $ENV{LIBSIGCPP_DIR}/include/sigc++-2.0
			-I $ENV{LIBSIGCPP_DIR}/lib/sigc++-2.0/include )
# this is how we do the genreflex step
macro (build_plugin maindir subdir)
  add_library(${maindir}${subdir}_dict_plugin SHARED ${maindir}${subdir}_dict_plugin.cpp )
  add_library(${maindir}${subdir}_map_plugin SHARED ${maindir}${subdir}_map_plugin.cpp )
  add_custom_command( 
     OUTPUT ${maindir}${subdir}_dict_plugin.cpp
     COMMAND ${GENREFLEX} ${CMAKE_SOURCE_DIR}/${maindir}/${subdir}/src/classes.h
        	 -s ${CMAKE_SOURCE_DIR}/${maindir}/${subdir}/src/classes_def.xml 
		 -I ${CMAKE_SOURCE_DIR}
		 -I ${CMAKE_SOURCE_DIR}/${maindir}/${subdir}/src
		 ${GENREFLEX_INCLUDES}
        	 -o ${maindir}${subdir}_dict_plugin.cpp  
		 ${GENREFLEX_FLAGS}
  )
  add_custom_command( 
     OUTPUT ${maindir}${subdir}_map_plugin.cpp
     DEPENDS ${maindir}${subdir}_dict_plugin.cpp
     COMMAND mv classes_ids.cc ${maindir}${subdir}_map_plugin.cpp
  )
  install ( TARGETS ${maindir}${subdir}_dict_plugin DESTINATION lib )
  install ( TARGETS ${maindir}${subdir}_map_plugin  DESTINATION lib )
endmacro (build_plugin maindir subdir)

# simple plugin libraries
macro (simple_plugin name)
  add_library(${name} SHARED ${name}.cc )
  install( TARGETS ${name}  DESTINATION lib )
endmacro (simple_plugin name)

# subdirectories
# cmake will automatically order the library builds according to declared dependencies
subdirs (art/Version)
subdirs (art/Utilities)
