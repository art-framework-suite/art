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

cet_set_compiler_flags(DIAGS VIGILANT WERROR NO_UNDEFINED EXTRA_FLAGS -pedantic)
cet_have_qual(e2 E2)
if ( E2 )
  cet_add_compiler_flags(CXX -std=c++0x)
endif()
cet_have_qual(sse2 SSE2)
if ( SSE2 )
  cet_add_compiler_flags(CXX -msse2 -ftree-vectorizer-verbose=2)
endif()

cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_ups_product( cppunit v1_12_1 )
find_ups_product( gccxml v0_9 )
find_ups_product( libsigcpp v2_2_10 )
find_ups_product( clhep v2_1_2_2 )
find_ups_product( cetbuildtools v3_03_00 )
find_ups_product( messagefacility v1_10_00 )
find_ups_product( sqlite v3_07_08_00 )
find_ups_root( v5_34_00 )
find_ups_product( tbb v4_1 )

cet_check_gcc()

# include search path
# notice use of ENV to invoke an external variable
include_directories ( $ENV{LIBSIGCPP_INC}/sigc++-2.0 )
include_directories ( $ENV{LIBSIGCPP_LIB}/sigc++-2.0/include )

# define CLHEP
cet_find_library( CLHEP  NAMES CLHEP   PATHS ENV CLHEP_LIB_DIR NO_DEFAULT_PATH )

# have to make sure cmake doesn't select the system libraries
# define libsigc
cet_find_library( SIGC    NAMES sigc-2.0  PATHS ENV LIBSIGCPP_LIB NO_DEFAULT_PATH )
# define cppunit
cet_find_library( CPPUNIT NAMES cppunit   PATHS ENV CPPUNIT_LIB   NO_DEFAULT_PATH )

# SQLite
cet_find_library(SQLITE3 NAMES sqlite3 PATHS ENV SQLITE_LIB NO_DEFAULT_PATH)

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
