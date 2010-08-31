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
	DESTINATION include/FWCore/MessageService/interface )
>>><<<
# project name
project(FWCoreMessageServices)

# version number
set (FWCoreMessageServices_VERSION_MAJOR 1)
set (FWCoreMessageServices_VERSION_MINOR 0)

set( FWCoreMessageServices_sources ELadministrator.cc
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

link_libraries( DataFormatsProvenance  
		FWCoreServiceRegistry
		FWCoreMessageLogger
		FWCoreParameterSet
		FWCoreUtilities
                ${Boost_THREAD_LIBRARY} )

add_library(FWCoreMessageServices SHARED ${FWCoreMessageServices_sources} )

simple_plugin(MessageLogger_plugin)
simple_plugin(MessageServicePresence_plugin)
simple_plugin(SingleThreadMSPresence_plugin)

install ( TARGETS FWCoreMessageServices DESTINATION lib )
