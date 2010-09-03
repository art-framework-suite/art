# project name
project(artFrameworkServicesMessage)

set( artFrameworkServicesMessage_sources ELadministrator.cc
				   ELcollected.cc
				   ELdestControl.cc
				   ELdestination.cc
				   ELerrorList.cc
				   ELfwkJobReport.cc
				   ELlimitsTable.cc
				   ELoutput.cc
				   ELrecv.cc
				   ELstatistics.cc
				   ELtsErrorLog.cc
				   ErrorLog.cc
				   HardwiredDefaults.cc
				   MessageLogger.cc
				   MessageLoggerDefaults.cc
				   MessageLoggerScribe.cc
				   MessageServicePresence.cc
				   MsgContext.cc
				   SingleThreadMSPresence.cc )

#link_libraries( artPersistencyProvenance
#		artFrameworkServicesRegistry
#		artMessageLogger
#		artParameterSet
#		artUtilities
#                ${Boost_THREAD_LIBRARY} )

add_library(artFrameworkServicesMessage SHARED ${artFrameworkServicesMessage_sources} )

simple_plugin(MessageLogger_plugin)
simple_plugin(MessageServicePresence_plugin)
simple_plugin(SingleThreadMSPresence_plugin)

install ( TARGETS artFrameworkServicesMessage DESTINATION lib )
install(FILES ELadministrator.h
	      ELassertN.h
	      ELcollected.h
	      ELcontextSupplier.h
	      ELdestControl.h
	      ELdestination.h
	      ELerrorList.h
	      ELfwkJobReport.h
	      ELlimitsTable.h
	      ELoutput.h
	      ELrecv.h
	      ELsender.h
	      ELset.h
	      ELstatistics.h
	      ELtsErrorLog.h
	      ELtsErrorLog.icc
	      ErrorLog.h
	      ErrorLog.icc
	      MessageLoggerDefaults.h
	      MessageLogger.h
	      MessageLoggerScribe.h
	      MessageServicePresence.h
	      MsgContext.h
	      NamedDestination.h
	      SingleThreadMSPresence.h
	      ThreadSafeErrorLog.h
	DESTINATION include/art/Framework/Services/Message )
