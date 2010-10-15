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
set( product art )
set( version v0_0_0 )
SET (qualifier a1)

# sl5 is 2.6.18
# sl4 is 2.6.9
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
   if( ${CMAKE_HOST_SYSTEM_VERSION} MATCHES "^2.6.9.*" )
       SET( SLTYPE sl4 )
   endif ()
   if( ${CMAKE_HOST_SYSTEM_VERSION} MATCHES "^2.6.18.*" )
       SET( SLTYPE sl5 )
   endif ()
   message(STATUS "Building on Linux ${SLTYPE} ${CMAKE_SYSTEM_PROCESSOR}" )
endif ()
# require SLTYPE
if ( NOT SLTYPE )
  message(FATAL_ERROR "Can't determine system type")
endif ()
SET (flavorqual ${SLTYPE}.${CMAKE_SYSTEM_PROCESSOR}.${qualifier})
SET (flavorqual_dir ${product}/${version}/${flavorqual} )

# because we want to move these libraries about,
# do not embed full path in shared libraries or executables
set (CMAKE_SKIP_RPATH)
# get verbose output so we can debug cmake
#set (CMAKE_VERBOSE_MAKEFILE TRUE)

ENABLE_TESTING()

# require ART_HOME
set( ART_HOME $ENV{ART_HOME} )
if ( NOT ART_HOME )
  message(FATAL_ERROR "Can't locate ART_HOME, required for building framework")
endif ()

# require CPPUNIT_VERSION
set( CPPUNIT_VERSION $ENV{CPPUNIT_VERSION} )
if ( NOT CPPUNIT_VERSION )
  message(FATAL_ERROR "cppunit has not been setup")
endif ()
SET ( CPPUNIT_STRING $ENV{SETUP_CPPUNIT} )
STRING( REGEX REPLACE ".*([-][q]+ )(.*)([-][-j])" "\\2" CPPUNIT_QUAL "${CPPUNIT_STRING}" )
message(STATUS "CPPUNIT version and qualifier are ${CPPUNIT_VERSION} ${CPPUNIT_QUAL}" )

# require GCCXML_VERSION
set( GCCXML_VERSION $ENV{GCCXML_VERSION} )
if ( NOT GCCXML_VERSION )
  message(FATAL_ERROR "gccxml has not been setup")
endif ()
SET ( GCCXML_STRING $ENV{SETUP_GCCXML} )
STRING( REGEX REPLACE ".*([-][q]+ )(.*)([-][-j])" "\\2" GCCXML_QUAL "${GCCXML_STRING}" )
message(STATUS "GCCXML version and qualifier are ${GCCXML_VERSION} ${GCCXML_QUAL}" )

# require CPP0X_VERSION
set( CPP0X_VERSION $ENV{CPP0X_VERSION} )
if ( NOT CPP0X_VERSION )
  message(FATAL_ERROR "CPP0X has not been setup")
endif ()
SET ( CPP0X_STRING $ENV{SETUP_CPP0X} )
STRING( REGEX REPLACE ".*([-][q]+ )(.*)([-][-j])" "\\2" CPP0X_QUAL "${CPP0X_STRING}" )
message(STATUS "CPP0X version and qualifier are ${CPP0X_VERSION} ${CPP0X_QUAL}" )

# require CETLIB_VERSION
set( CETLIB_VERSION $ENV{CETLIB_VERSION} )
if ( NOT CETLIB_VERSION )
  message(FATAL_ERROR "CETLIB has not been setup")
endif ()
SET ( CETLIB_STRING $ENV{SETUP_CETLIB} )
STRING( REGEX REPLACE ".*([-][q]+ )(.*)([-][-j])" "\\2" CETLIB_QUAL "${CETLIB_STRING}" )
message(STATUS "CETLIB version and qualifier are ${CETLIB_VERSION} ${CETLIB_QUAL}" )

# require LIBSIGCPP_VERSION
set( LIBSIGCPP_VERSION $ENV{LIBSIGCPP_VERSION} )
if ( NOT LIBSIGCPP_VERSION )
  message(FATAL_ERROR "libsigcpp has not been setup")
endif ()
SET ( LIBSIGCPP_STRING $ENV{SETUP_LIBSIGCPP} )
STRING( REGEX REPLACE ".*([-][q]+ )(.*)([-][-j])" "\\2" LIBSIGCPP_QUAL "${LIBSIGCPP_STRING}" )
message(STATUS "LIBSIGCPP version and qualifier are ${LIBSIGCPP_VERSION} ${LIBSIGCPP_QUAL}" )

# require CLHEP_VERSION
set( CLHEP_VERSION $ENV{CLHEP_VERSION} )
if ( NOT CLHEP_VERSION )
  message(FATAL_ERROR "clhep has not been setup")
endif ()
SET ( CLHEP_STRING $ENV{SETUP_CLHEP} )
STRING( REGEX REPLACE ".*([-][q]+ )(.*)([-][-j])" "\\2" CLHEP_QUAL "${CLHEP_STRING}" )
message(STATUS "CLHEP version and qualifier are ${CLHEP_VERSION} ${CLHEP_QUAL}" )

# require ROOTSYS
set( ROOTSYS $ENV{ROOTSYS} )
if ( NOT ROOTSYS )
  message(FATAL_ERROR "root has not been setup")
endif ()
SET ( ROOT_STRING $ENV{SETUP_ROOT} )
STRING( REGEX REPLACE ".*([-][q]+ )(.*)([^-][^j]).*" "\\2" ROOT_QUAL "${ROOT_STRING}" )
STRING( REGEX REPLACE "^[r][o][o][t][ ]+([^ ]+).*" "\\1" ROOT_VERSION "${ROOT_STRING}" )
message(STATUS "ROOT version and qualifier are ${ROOT_VERSION} ${ROOT_QUAL}" )

# require FHICLCPP_VERSION
set( FHICLCPP_VERSION $ENV{FHICLCPP_VERSION} )
if ( NOT FHICLCPP_VERSION )
  message(FATAL_ERROR "fhiclcpp has not been setup")
endif ()
SET ( FHICLCPP_STRING $ENV{SETUP_FHICLCPP} )
STRING( REGEX REPLACE ".*([-][q]+ )(.*)([-][-j])" "\\2" FHICLCPP_QUAL "${FHICLCPP_STRING}" )
message(STATUS "FHICLCPP version and qualifier are ${FHICLCPP_VERSION} ${FHICLCPP_QUAL}" )

# require MESSAGEFACILITY_VERSION
set( MESSAGEFACILITY_VERSION $ENV{MESSAGEFACILITY_VERSION} )
if ( NOT MESSAGEFACILITY_VERSION )
  message(FATAL_ERROR "messagefacility has not been setup")
endif ()
SET ( MESSAGEFACILITY_STRING $ENV{SETUP_MESSAGEFACILITY} )
STRING( REGEX REPLACE ".*([-][q]+ )(.*)([-][-j])" "\\2" MESSAGEFACILITY_QUAL "${MESSAGEFACILITY_STRING}" )
message(STATUS "MESSAGEFACILITY version and qualifier are ${MESSAGEFACILITY_VERSION} ${MESSAGEFACILITY_QUAL}" )

# Check if the boost library has been set
# boost is a special case
SET ( BOOST_VERS $ENV{BOOST_VERSION} )
IF (NOT BOOST_VERS)
    MESSAGE (FATAL_ERROR "Boost library has not been setup")
ENDIF()
SET ( BOOST_STRING $ENV{SETUP_BOOST} )
STRING( REGEX REPLACE ".*([-][q]+ )(.*)([-][-j])" "\\2" BOOST_QUAL "${BOOST_STRING}" )
message(STATUS "Boost version and qualifier are ${BOOST_VERS} ${BOOST_QUAL}" )

# make sure gcc has been set
# note that gcc has no qualifier
SET ( GCC_VERSION $ENV{GCC_VERSION} )
IF (NOT GCC_VERSION)
    MESSAGE (FATAL_ERROR "gcc has not been setup")
ENDIF()
message(STATUS "GCC version is ${GCC_VERSION} " )



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
include_directories ( $ENV{FHICLCPP_INC} )
include_directories ( $ENV{MESSAGEFACILITY_INC} )

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

# define FHICLCPP
find_library( FHICLCPP NAMES fhiclcpp PATHS $ENV{FHICLCPP_LIB} )

# define MESSAGEFACILITY
find_library( MESSAGEFACILITY NAMES MF_MessageLogger
                                    MF_MessageService
                                    MF_Utilities
                                    MF_Extensions
                              PATHS $ENV{MESSAGEFACILITY_LIB} )

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
			-I $ENV{FHICLCPP_INC}
			-I $ENV{MESSAGEFACILITY_INC}
			-I $ENV{LIBSIGCPP_DIR}/include/sigc++-2.0
			-I $ENV{LIBSIGCPP_DIR}/lib/sigc++-2.0/include )
# this is how we do the genreflex step
macro (build_dictionary maindir subdir)
  add_library(${maindir}${subdir}_dict_plugin SHARED ${maindir}${subdir}_dict_plugin.cpp )
  add_library(${maindir}${subdir}_map_plugin SHARED ${maindir}${subdir}_map_plugin.cpp )
  set(dictionary_liblist "${ARGN}")
  if( dictionary_liblist )
    target_link_libraries( ${maindir}${subdir}_dict_plugin ${dictionary_liblist} )
    target_link_libraries( ${maindir}${subdir}_map_plugin ${dictionary_liblist} )
  endif( dictionary_liblist )
  add_custom_command(
     OUTPUT ${maindir}${subdir}_dict_plugin.cpp
     COMMAND ${GENREFLEX} ${CMAKE_SOURCE_DIR}/art/${maindir}/${subdir}/classes.h
        	 -s ${CMAKE_SOURCE_DIR}/art/${maindir}/${subdir}/classes_def.xml
		 -I ${CMAKE_SOURCE_DIR}
		 -I ${CMAKE_SOURCE_DIR}/art/${maindir}/${subdir}
		 ${GENREFLEX_INCLUDES}
        	 -o ${maindir}${subdir}_dict_plugin.cpp
		 ${GENREFLEX_FLAGS} || { rm -f ${maindir}${subdir}_dict_plugin.cpp\; /bin/false\; }
     DEPENDS ${CMAKE_SOURCE_DIR}/art/${maindir}/${subdir}/src/classes.h
             ${CMAKE_SOURCE_DIR}/art/${maindir}/${subdir}/src/classes_def.xml
  )
  add_custom_command(
     OUTPUT ${maindir}${subdir}_map_plugin.cpp
     DEPENDS ${maindir}${subdir}_dict_plugin.cpp
     COMMAND mv classes_ids.cc ${maindir}${subdir}_map_plugin.cpp
  )
  install ( TARGETS ${maindir}${subdir}_dict_plugin DESTINATION ${flavorqual_dir}/lib )
  install ( TARGETS ${maindir}${subdir}_map_plugin  DESTINATION ${flavorqual_dir}/lib )
endmacro (build_dictionary maindir subdir)

# simple plugin libraries
macro (simple_plugin name)
  add_library(${name} SHARED ${name}.cc )
  set(simple_plugin_liblist "${ARGN}")
  if( simple_plugin_liblist )
    target_link_libraries( ${name} ${simple_plugin_liblist} )
  endif( simple_plugin_liblist )
  install( TARGETS ${name}  DESTINATION ${flavorqual_dir}/lib )
endmacro (simple_plugin name)

# source
subdirs (art)

# testing
subdirs(test)

# documentation
subdirs(doc)

# ups - table and config files
subdirs(ups)
