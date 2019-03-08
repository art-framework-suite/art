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


# use cmake 3.11 or later
cmake_minimum_required (VERSION 3.11)

project(art VERSION 3.02.03 LANGUAGES CXX)

# cetmodules contains our cmake modules
find_package(cetmodules REQUIRED)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Modules)

include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS VIGILANT
  WERROR
  NO_UNDEFINED
  EXTRA_FLAGS -pedantic
  EXTRA_CXX_FLAGS -Wnon-virtual-dtor -Wdelete-non-virtual-dtor)

cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions

find_package(hep_concurrency REQUIRED)
find_package(cetlib_except REQUIRED)
find_package(cetlib REQUIRED)
find_package(canvas REQUIRED)
find_package(fhiclcpp REQUIRED)
find_package(messagefacility REQUIRED)
find_package(CLHEP REQUIRED COMPONENTS Matrix Vector Random)
find_package(Boost REQUIRED
  COMPONENTS date_time filesystem graph program_options
  regex system unit_test_framework)
find_package(TBB REQUIRED)

find_library(SQLITE3 NAMES sqlite3 REQUIRED)

# macros for simple_plugin
include(BuildPlugins)

# Plugin skeleton generators for cetskelgen.
add_subdirectory(perllib)

# tools
add_subdirectory(tools)

# source
add_subdirectory(art)

# CMake modules
add_subdirectory(Modules)

# ups - table and config files
add_subdirectory(cmakeConfig)

