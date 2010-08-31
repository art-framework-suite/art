install(FILES AbstractMLscribe.h
	      ConfigurationHandshake.h
	      ELextendedID.h
	      ELlist.h
	      ELmap.h
	      ELseverityLevel.h
	      ELseverityLevel.icc
	      ELstring.h
	      ErrorObj.h
	      ErrorObj.icc
	      ErrorSummaryEntry.h
	      ExceptionMessages.h
	      LoggedErrorsSummary.h
	      MessageDrop.h
	      MessageLogger.h
	      MessageLoggerQ.h
	      MessageSender.h
	      ThreadSafeErrorLog.icc
	DESTINATION include/FWCore/MessageLogger/interface )
>>><<<
# project name
project(FWCoreMessageLogger)

# version number
set (FWCoreMessageLogger_VERSION_MAJOR 1)
set (FWCoreMessageLogger_VERSION_MINOR 0)

set( FWCoreMessageLogger_sources AbstractMLscribe.cc
				 ELextendedID.cc
				 ELmap.cc
				 ELseverityLevel.cc
				 ELstring.cc
				 ErrorObj.cc
				 ExceptionMessages.cc
				 LoggedErrorsSummary.cc
				 MessageDrop.cc
				 MessageLogger.cc
				 MessageLoggerQ.cc
				 MessageSender.cc )

add_library(FWCoreMessageLogger SHARED ${FWCoreMessageLogger_sources} )
target_link_libraries(FWCoreMessageLogger FWCoreUtilities
                			  ${Boost_FILESYSTEM_LIBRARY} 
                			  ${Boost_THREAD_LIBRARY} 
					  ${CORE} 
					  ${HIST} 
					  ${REFLEX} 
					  ${CINTEX} )

install ( TARGETS FWCoreMessageLogger DESTINATION lib )
