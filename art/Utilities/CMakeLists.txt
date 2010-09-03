# project name
project(artUtilities)

set( artUtilities_sources CPUTimer.cc
		       CRC32Calculator.cc
		       DebugMacros.cc
		       Digest.cc
		       EDMException.cc
		       Exception.cc
		       ExceptionCollector.cc
		       FriendlyName.cc
		       GlobalIdentifier.cc
		       GlobalMutex.cc
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

link_libraries( ${Boost_FILESYSTEM_LIBRARY}
                ${Boost_REGEX_LIBRARY}
		${Boost_THREAD_LIBRARY}
		${REFLEX}
		${CINT}
		dl )

add_library(artUtilities SHARED ${artUtilities_sources} )

install ( TARGETS artUtilities DESTINATION lib )
install(FILES Algorithms.h
	      CodedException.h
	      CPUTimer.h
	      CRC16.h
	      CRC32Calculator.h
	      DebugMacros.h
	      Digest.h
	      ECGetterBase.h
	      EDMException.h
	      ExceptionCollector.h
	      Exception.h
	      ExtensionCord.h
	      FriendlyName.h
	      GCCPrerequisite.h
	      GetEnvironmentVariable.h
	      GetPassID.h
	      GetReleaseVersion.h
	      GlobalIdentifier.h
	      GlobalMutex.h
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
	      value_ptr.h
	      vectorTransform.h
	      Verbosity.h
	      WrappedClassName.h
	DESTINATION include/art/Utilities )
