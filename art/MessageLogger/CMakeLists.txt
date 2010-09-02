# project name
project(artMessageLogger)

set( artMessageLogger_sources AbstractMLscribe.cc
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

add_library(artMessageLogger SHARED ${artMessageLogger_sources} )
link_libraries( artUtilities
                ${Boost_FILESYSTEM_LIBRARY}
                ${Boost_THREAD_LIBRARY}
		${CORE}
		${HIST}
		${REFLEX}
		${CINTEX} )

install ( TARGETS artMessageLogger DESTINATION lib )
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
	DESTINATION include/art/MessageLogger )
