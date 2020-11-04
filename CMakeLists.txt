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


cmake_minimum_required(VERSION 3.18.2 FATAL_ERROR)
cmake_policy(VERSION 3.14)
# Policy CMP0096 must be set to NEW to preserve leading zeros in the
# VERSION set by project().
cmake_policy(SET CMP0096 NEW)
find_package(cetmodules 2.00.02 REQUIRED)
project(art VERSION 3.06.03 LANGUAGES CXX C)

# cetbuildtools contains our cmake modules ### MIGRATE-ACTION-REQUIRED: remove

list(APPEND CMAKE_MODULE_PATH ### MIGRATE-ACTION-REQUIRED: remove
  ${CMAKE_CURRENT_SOURCE_DIR}/Modules)

include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS VIGILANT
  WERROR
  NO_UNDEFINED
  EXTRA_FLAGS -pedantic
  EXTRA_CXX_FLAGS -Wnon-virtual-dtor -Wdelete-non-virtual-dtor -DBOOST_TEST_NO_OLD_TOOLS)

cet_have_qual(sse2 SSE2)
if (SSE2)
  cet_add_compiler_flags(CXX -msse2 -ftree-vectorizer-verbose=2)
endif()

if($ENV{CET_SANITIZE_ADDRESS})
  if(NOT "$ENV{GCC_FQ_DIR}" STREQUAL "")
    find_file(ASAN_LIB libasan.so NO_DEFAULT_PATH PATHS $ENV{GCC_FQ_DIR}/lib64)
  endif()
endif()

if($ENV{CET_SANITIZE_THREAD})
  if(NOT "$ENV{GCC_FQ_DIR}" STREQUAL "")
    find_file(TSAN_LIB libtsan.so NO_DEFAULT_PATH PATHS $ENV{GCC_FQ_DIR}/lib64)
  endif()
endif()

if ($ENV{CET_SANITIZE_ADDRESS})
cet_add_compiler_flags(CXX -fsanitize=address)
cet_add_compiler_flags(C -fsanitize=address)
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address")
set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} -fsanitize=address")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=address")
endif()

if ($ENV{CET_SANITIZE_THREAD})
cet_add_compiler_flags(CXX -fsanitize=thread -D__SANITIZE_THREAD__)
cet_add_compiler_flags(C -fsanitize=thread -D__SANITIZE_THREAD__)
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=thread -D__SANITIZE_THREAD__")
set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} -fsanitize=thread -D__SANITIZE_THREAD__")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=thread -D__SANITIZE_THREAD__")
endif()

cet_report_compiler_flags(REPORT_THRESHOLD VERBOSE)

# these are minimum required versions, not the actual product versions
find_ups_product(canvas v1_06_00) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking
find_ups_product(messagefacility) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking
find_ups_product(fhiclcpp) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking
find_ups_product(cetlib v2_01_00) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking
find_ups_product(cetlib_except v1_01_00) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking
find_ups_product(clhep) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking
find_ups_product(hep_concurrency) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking
find_ups_product(sqlite) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking
find_ups_product(range) ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking
find_ups_boost(BOOST_TARGETS)

# SQLite
cet_find_library(SQLITE3 NAMES sqlite3_ups PATHS ENV SQLITE_LIB NO_DEFAULT_PATH) ### MIGRATE-ACTION-RECOMMENDED: use of ENV ... may be UPS-dependent ### MIGRATE-ACTION-RECOMMENDED: use cet_find_package() with target semantics for linking

# macros for simple_plugin
include(BuildPlugins)

# Plugin skeleton generators for cetskelgen.
add_subdirectory(perllib)

# tools
add_subdirectory(tools)

# source
add_subdirectory(art)

# ups - table and config files

# CMake modules
add_subdirectory(Modules)

# packaging utility
cet_cmake_config(COMPATIBILITY SameMajorVersion)
