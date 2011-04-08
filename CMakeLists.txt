# ======================================================================
#  art main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/art/ups/setup_for_development
#  cmake [-DCMAKE_INSTALL_PREFIX=/install/path] [-Dqualifier:STRING=nova] $ART_SOURCE
#  make
#  make test
#  make install
#  make package
# ======================================================================


# use cmake 2.6 or later
cmake_minimum_required (VERSION 2.6)
# project name
project(art)
set( product art )
set( version v0_06_01 )
if ( NOT qualifier )
  SET (qualifier a2)
endif ( NOT qualifier )

# cetbuildtools contains our cmake modules
SET ( CETBUILDTOOLS_VERSION $ENV{CETBUILDTOOLS_VERSION} )
IF (NOT CETBUILDTOOLS_VERSION)
    MESSAGE (FATAL_ERROR "ERROR: setup cetbuildtools to get the cmake modules")
ENDIF()
set(CMAKE_MODULE_PATH "${CMAKE_ROOT}/Modules")
set(CMAKE_MODULE_PATH $ENV{CETBUILDTOOLS_DIR}/Modules ${CMAKE_MODULE_PATH})
message(STATUS "module path is ${CMAKE_MODULE_PATH}" )

# because we want to move these libraries about,
# do not embed full path in shared libraries or executables
set (CMAKE_SKIP_RPATH)
# get verbose output so we can debug cmake
#set (CMAKE_VERBOSE_MAKEFILE TRUE)

ENABLE_TESTING()

# make sure someone isn't typing cmake in the source code tree
include(EnsureOutOfSourceBuild)
cet_ensure_out_of_source_build()

# define some necessary modules
include(FindUpsPackage)
include(FindUpsBoost)
include(FindUpsRoot)
include(ParseUpsVersion)
include(SetFlavorQual)
include(InstallSource)

#set package version from ups version
set_version_from_ups( ${version} )
#define flavorqual and flavorqual_dir
set_flavor_qual()


# require ART_BUILD
set( ART_BUILD $ENV{ART_BUILD} )
if ( NOT ART_BUILD )
  message(FATAL_ERROR "Can't locate ART_BUILD, required for building framework")
endif ()

# these are minimum required versions, not the actual product versions
find_ups_product( cppunit v1_12_1 )
find_ups_product( gccxml v0_9 )
find_ups_product( libsigcpp v2_2_7 )
find_ups_product( clhep v2_1_0_0 )
find_ups_product( fhiclcpp v2_12_00 )
find_ups_product( messagefacility v1_8_12 )
find_ups_product( cetlib v0_05_03 )
find_ups_product( cpp0x v1_01_01 )
find_ups_product( cetbuildtools v1_10_02 )
find_ups_root( v5_28_00_p02 )
find_ups_boost( v1_45_0 filesystem
                        program_options
                        regex
                        thread
                        unit_test_framework
 )

# make sure gcc has been set
# note that gcc has no qualifier
SET ( GCC_VERSION $ENV{GCC_VERSION} )
IF (NOT GCC_VERSION)
    MESSAGE (FATAL_ERROR "gcc has not been setup")
ENDIF()
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
message(STATUS "GCC version is ${GCC_VERSION} with ${CMAKE_CXX_FLAGS}")

# include search path
include_directories ("${PROJECT_BINARY_DIR}")
include_directories ("${PROJECT_SOURCE_DIR}")
# notice use of ENV to invoke an external variable
include_directories ( $ENV{LIBSIGCPP_INC}/sigc++-2.0 )
include_directories ( $ENV{LIBSIGCPP_LIB}/sigc++-2.0/include )

# define CLHEP
find_library( CLHEP  NAMES CLHEP   PATHS $ENV{CLHEP_BASE}/lib )

# define CETLIB
find_library( CETLIB NAMES cetlib PATHS $ENV{CETLIB_LIB} )

# define FHICLCPP
find_library( FHICLCPP NAMES fhiclcpp PATHS $ENV{FHICLCPP_LIB} )

# define MESSAGEFACILITY
find_library( MF_MESSAGELOGGER NAMES MF_MessageLogger
                              PATHS $ENV{MESSAGEFACILITY_LIB} )
find_library( MF_MESSAGESERVICE NAMES MF_MessageService
                              PATHS $ENV{MESSAGEFACILITY_LIB} )
find_library( MF_UTILITIES NAMES MF_Utilities
                              PATHS $ENV{MESSAGEFACILITY_LIB} )
find_library( MF_EXTENSIONS NAMES MF_ExtModules
                              PATHS $ENV{MESSAGEFACILITY_LIB} )

# have to make sure cmake doesn't select the system libraries
# define libsigc
find_library( SIGC    NAMES sigc-2.0  PATHS $ENV{LIBSIGCPP_LIB} NO_DEFAULT_PATH )
# define cppunit
find_library( CPPUNIT NAMES cppunit   PATHS $ENV{CPPUNIT_LIB}   NO_DEFAULT_PATH )


# define common flags
# set( CMAKE_CXX_LINK_FLAGS "${CPPUNIT} -luuid -ldl" )

#build all libraries in a single directory to enable testing
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
#build all executables in a single directory
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# macros for build_dictionary and simple_plugin
include(BuildDictionary)
include(BuildPlugins)

# tools (first)
subdirs(tools)

# source
subdirs (art)

# testing
subdirs(test)

# ups - table and config files
subdirs(ups)

# packaging utility
include(UseCPack)
