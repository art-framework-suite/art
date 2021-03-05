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

find_package(cetmodules 2.04.00 REQUIRED)
project(art VERSION 3.07.00 LANGUAGES CXX C)

include(CetCMakeEnv)
cet_cmake_env()

# Find our own CMake modules, and tell everyone else where to find them.
cet_cmake_module_directories(Modules)

cet_set_compiler_flags(DIAGS VIGILANT
  WERROR
  NO_UNDEFINED
  EXTRA_FLAGS -pedantic)

if(DEFINED ENV{CET_SANITIZE_ADDRESS} AND DEFINED ENV{GCC_FQ_DIR})
  find_file(ASAN_LIB libasan.so NO_DEFAULT_PATH PATHS $ENV{GCC_FQ_DIR}/lib64)
  cet_add_compiler_flags(CXX -fsanitize=address)
  cet_add_compiler_flags(C -fsanitize=address)
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address")
  set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} -fsanitize=address")
  set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=address")
endif()

if(DEFINED ENV{CET_SANITIZE_THREAD} AND DEFINED ENV{GCC_FQ_DIR})
  find_file(TSAN_LIB libtsan.so NO_DEFAULT_PATH PATHS $ENV{GCC_FQ_DIR}/lib64)
  cet_add_compiler_flags(CXX -fsanitize=thread -D__SANITIZE_THREAD__)
  cet_add_compiler_flags(C -fsanitize=thread -D__SANITIZE_THREAD__)
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=thread -D__SANITIZE_THREAD__")
  set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} -fsanitize=thread -D__SANITIZE_THREAD__")
  set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=thread -D__SANITIZE_THREAD__")
endif()

cet_report_compiler_flags(REPORT_THRESHOLD VERBOSE)

cet_find_package(canvas REQUIRED)
cet_find_package(messagefacility REQUIRED)
cet_find_package(fhiclcpp REQUIRED)
cet_find_package(cetlib REQUIRED)
cet_find_package(cetlib_except REQUIRED)
cet_find_package(CLHEP REQUIRED)
cet_find_package(hep_concurrency REQUIRED)
cet_find_package(Range REQUIRED)
cet_find_package(Catch2 REQUIRED PRIVATE)
cet_find_package(Boost COMPONENTS date_time filesystem program_options regex unit_test_framework REQUIRED)
cet_find_package(TBB REQUIRED)
cet_find_package(SQLite3 REQUIRED)

cet_find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
add_subdirectory(third_party/RapidJSON)

# macros for simple_plugin
include(BuildPlugins)
list(PREPEND CMAKE_PREFIX_PATH ${PROJECT_BINARY_DIR}/third_party/RapidJSON/rapidjson)

# Plugin skeleton generators for cetskelgen.
add_subdirectory(perllib)

# tools
add_subdirectory(tools)

# source
add_subdirectory(art)

# CMake modules
add_subdirectory(Modules)

cet_cmake_config(COMPATIBILITY SameMajorVersion)
