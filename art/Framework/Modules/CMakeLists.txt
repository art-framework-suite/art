# project name
project(artFrameworkModules)

set( artFrameworkModules_sources AsciiOutputModule.cc
			   EmptyESSource.cc
			   EmptySource.cc
			   EventContentAnalyzer.cc
			   EventSetupRecordDataGetter.cc
			   IterateNTimesLooper.cc
			   Prescaler.cc
			   PrintEventSetupContent.cc
			   ProvenanceCheckerOutputModule.cc
			   XMLOutputModule.cc )


link_libraries( DataFormatsProvenance
                artFrameworkCore
                artParameterSet
 		${Boost_FILESYSTEM_LIBRARY}
		${REFLEX} )

add_library(artFrameworkModules SHARED ${artFrameworkModules_sources} )

simple_plugin(AsciiOutputModule_plugin)
simple_plugin(EmptyESSource_plugin)
simple_plugin(EmptySource_plugin)
simple_plugin(EventContentAnalyzer_plugin)
simple_plugin(EventSetupRecordDataGetter_plugin)
simple_plugin(IterateNTimesLooper_plugin)
simple_plugin(Prescaler_plugin)
simple_plugin(RandomNumberSaver_plugin)
simple_plugin(XMLOutputModule_plugin)

install ( TARGETS artFrameworkModules DESTINATION lib )
