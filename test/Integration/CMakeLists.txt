# project name
project(art_Integration_Test)

#set( art_Integration_Test_sources
#
#   )

#add_library(art_Integration_Test SHARED ${art_Integration_Test_sources} )

simple_plugin(IntProducer "plugin")
simple_plugin(IntTestAnalyzer "plugin")

#install ( TARGETS art_Integration_Test
#          DESTINATION ${flavorqual_dir}/lib )
