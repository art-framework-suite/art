# project name
project(art_Utilities)

set( art_Utilities_sources CPUTimer.cc
                          CRC32Calculator.cc
                          DebugMacros.cc
                          Digest.cc
                          Exception.cc
                          ExceptionMessages.cc
                          FirstAbsoluteOrLookupWithDotPolicy.cc
                          FriendlyName.cc
                          InputTag.cc
                          MallocOpts.cc
                          md5.c
                          Parse.cc
                          ReflexTools.cc
                          RegexMatch.cc
                          RootHandlers.cc
                          SingleConsumerQ.cc
                          TestHelper.cc
                          TypeID.cc
                          UnixSignalHandlers.cc
                          WrappedClassName.cc
                          ensureTable.cc
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
  CRC16.h
  CRC32Calculator.h
  DebugMacros.h
  Digest.h
  ECGetterBase.h
  Exception.h
  ExceptionMessages.h
  ExtensionCord.h
  FirstAbsoluteOrLookupWithDotPolicy.h
  FriendlyName.h
  GetEnvironmentVariable.h
  GetPassID.h
  GetReleaseVersion.h
  HRRealTime.h
  InputTag.h
  JobMode.h
  MallocOpts.h
  Math.h
  OutletBase.h
  Parse.h
  PretendToUse.h
  ReflexTools.h
  RegexMatch.h
  RootHandlers.h
  SimpleOutlet.h
  SingleConsumerQ.h
  TestHelper.h
  ThreadSafeIndexedRegistry.h
  TypeID.h
  TypeIDBase.h
  UnixSignalHandlers.h
  UseReflex.h
  Verbosity.h
  WrappedClassName.h
  ensureTable.h
  md5.h
  vectorTransform.h
)

set (DETAIL_HEADERS
  math_private.h
)

foreach (header ${DETAIL_HEADERS})
  set (PATH_DETAIL_HEADERS ${PATH_DETAIL_HEADERS} "detail/${header}")
endforeach (header ${DETAIL_HEADERS})

install (FILES ${PATH_DETAIL_HEADERS}
        DESTINATION ${product}/${version}/include/art/Utilities/detail )

install ( TARGETS art_Utilities DESTINATION ${flavorqual_dir}/lib )
install (FILES ${HEADERS}
         DESTINATION ${product}/${version}/include/art/Utilities )

install_source()
