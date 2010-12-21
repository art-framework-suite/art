# project name
project(art_Framework_Services_Basic)

#set( art_Framework_Services_Basic_sources Tracer.cc )
#add_library( art_Framework_Services_Basic SHARED ${art_Framework_Services_Basic_sources} )

simple_plugin(CurrentModule)
simple_plugin(FloatingPointControl)
simple_plugin(SimpleMemoryCheck)
simple_plugin(TFileService)
simple_plugin(Timing)
simple_plugin(Tracer)

#install( TARGETS art_Framework_Services_Basic
#         DESTINATION ${flavorqual_dir}/lib
#       )
install( FILES CurrentModule.h TFileService.h
         DESTINATION  ${product}/${version}/include/art/Framework/Services/Basic
       )
