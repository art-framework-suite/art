# project name
project(art_Utilities)

foreach(cfg_source
    LibraryManager.h
    LibraryManager.cc
    )
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${cfg_source}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${cfg_source} @ONLY
    )
endforeach()

set(art_Utilities_sources
  ${CMAKE_CURRENT_BINARY_DIR}/LibraryManager.cc
  CPUTimer.cc
  CRC32Calculator.cc
  DebugMacros.cc
  Digest.cc
  Exception.cc
  ExceptionMessages.cc
  FirstAbsoluteOrLookupWithDotPolicy.cc
  FriendlyName.cc
  InputTag.cc
  MallocOpts.cc
  RegexMatch.cc
  RootHandlers.cc
  TestHelper.cc
  TypeID.cc
  UnixSignalHandlers.cc
  WrappedClassName.cc
  ensureTable.cc
  md5.c
  )

add_library(art_Utilities SHARED ${art_Utilities_sources} )
target_link_libraries(art_Utilities
  ${MF_MESSAGELOGGER}
  ${CETLIB}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_REGEX_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  -luuid
)

set (HEADERS
  CPUTimer.h
  CRC32Calculator.h
  DebugMacros.h
  Digest.h
  Exception.h
  ExceptionMessages.h
  FirstAbsoluteOrLookupWithDotPolicy.h
  FriendlyName.h
  GetPassID.h
  GetReleaseVersion.h
  HRRealTime.h
  InputTag.h
  JobMode.h
  MallocOpts.h
  Math.h
  RegexMatch.h
  RootHandlers.h
  TestHelper.h
  ThreadSafeIndexedRegistry.h
  TypeID.h
  UnixSignalHandlers.h
  Verbosity.h
  WrappedClassName.h
  ensureTable.h
  md5.h
  vectorTransform.h
)

set (DETAIL_HEADERS
  math_private.h
  metaprogramming.h
)

foreach (header ${DETAIL_HEADERS})
  set (PATH_DETAIL_HEADERS ${PATH_DETAIL_HEADERS} "detail/${header}")
endforeach (header ${DETAIL_HEADERS})

install ( TARGETS art_Utilities DESTINATION ${flavorqual_dir}/lib )

install_headers(SUBDIRS detail)
install_source()
