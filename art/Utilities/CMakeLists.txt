# project name
project(art_Utilities)

set( art_Utilities_sources CPUTimer.cc
                          CRC32Calculator.cc
                          DebugMacros.cc
                          Digest.cc
                          Exception.cc
                          ExceptionMessages.cc
                          FriendlyName.cc
                          GlobalIdentifier.cc
                          Guid.cc
                          InputTag.cc
                          MallocOpts.cc
                          md5.c
                          Parse.cc
                          Presence.cc
                          ReflexTools.cc
                          RegexMatch.cc
                          RootHandlers.cc
                          SingleConsumerQ.cc
                          TestHelper.cc
                          TypeID.cc
                          UnixSignalHandlers.cc
                          WrappedClassName.cc )

#link_libraries( ${Boost_FILESYSTEM_LIBRARY}
#                ${Boost_REGEX_LIBRARY}
#                ${Boost_THREAD_LIBRARY}
#                ${REFLEX}
#                ${CINT}
#                dl )

add_library(art_Utilities SHARED ${art_Utilities_sources} )

install ( TARGETS art_Utilities DESTINATION ${flavorqual_dir}/lib )
install(FILES
              CPUTimer.h
              CRC16.h
              CRC32Calculator.h
              DebugMacros.h
              Digest.h
              ECGetterBase.h
              Exception.h
              ExceptionMessages.h
              ExtensionCord.h
              FriendlyName.h
              GetEnvironmentVariable.h
              GetPassID.h
              GetReleaseVersion.h
              GlobalIdentifier.h
              HRRealTime.h
              InputTag.h
              JobMode.h
              MallocOpts.h
              Math.h
              math_private.h
              md5.h
              OutletBase.h
              Parse.h
              Presence.h
              PretendToUse.h
              ReflexTools.h
              RegexMatch.h
              RootHandlers.h
              SimpleOutlet.h
              SingleConsumerQ.h
              TestHelper.h
              ThreadSafeIndexedRegistry.h
              ThreadSafeRegistry.h
              TypeIDBase.h
              TypeID.h
              UnixSignalHandlers.h
              UseReflex.h
              vectorTransform.h
              Verbosity.h
              WrappedClassName.h
        DESTINATION ${product}/${version}/include/art/Utilities )
