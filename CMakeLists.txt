# ======================================================================
#  art main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/art/ups/setup_for_development
#  cmake [-DCMAKE_INSTALL_PREFIX=/install/path]
#        -DCMAKE_BUILD_TYPE=$CETPKG_TYPE
#        $CETPKG_SOURCE
#  make
#  make test
#  make install
#  make package (builds distribution tarfile)
# ======================================================================


# use cmake 2.8 or later
cmake_minimum_required (VERSION 2.8)

project(art)

# cetbuildtools contains our cmake modules
SET ( CETBUILDTOOLS_VERSION $ENV{CETBUILDTOOLS_VERSION} )
IF (NOT CETBUILDTOOLS_VERSION)
    MESSAGE (FATAL_ERROR "ERROR: setup cetbuildtools to get the cmake modules")
ENDIF()
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Modules 
                      $ENV{CETBUILDTOOLS_DIR}/Modules 
		      ${CMAKE_MODULE_PATH})

include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS VIGILANT WERROR EXTRA_FLAGS -pedantic)
cet_have_qual( e2 )
if ( ${CET_HAVE_QUAL} )
  cet_add_compiler_flags(CXX -std=c++0x;-Wno-deprecated-declarations)
endif()
cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_ups_product( cppunit v1_12_1 )
find_ups_product( gccxml v0_9 )
find_ups_product( libsigcpp v2_2_10 )
find_ups_product( clhep v2_1_2_2 )

find_ups_product( cetbuildtools v2_01_00 )
find_ups_product( cpp0x v1_03_00 )
find_ups_product( cetlib v1_03_00 )
find_ups_product( fhiclcpp v2_16_00 )
find_ups_product( messagefacility v1_10_00 )
find_ups_product( sqlite v3_07_08_00 )
find_ups_root( v5_34_00 )
find_ups_boost( v1_47_0 filesystem
                        program_options
                        regex
                        thread
                        unit_test_framework
 )

cet_check_gcc()

# include search path
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

find_library(SQLITE3 NAMES sqlite3 PATHS $ENV{SQLITE_LIB} NO_DEFAULT_PATH)

# define common flags
# set( CMAKE_CXX_LINK_FLAGS "${CPPUNIT} -luuid -ldl" )

# macros for art_dictionary and simple_plugin
include(ArtDictionary)
include(BuildPlugins)

# tools (first)
add_subdirectory(tools)

# source
add_subdirectory (art)

# testing
add_subdirectory(test)

# ups - table and config files
add_subdirectory(ups)

# CMake modules
add_subdirectory(Modules)

# packaging utility
include(UseCPack)
