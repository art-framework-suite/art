# project name
project(artFrameworkIOCommon)

set( artFrameworkIOCommon_sources RootServiceChecker.cc )

#link_libraries( artUtilities
#                artFrameworkServicesRegistry )

add_library(artFrameworkIOCommon SHARED ${artFrameworkIOCommon_sources} )

install ( TARGETS artFrameworkIOCommon DESTINATION ${flavorqual_dir}/lib )

install(FILES ClassFiller.h
              RefStreamer.h
              RootServiceChecker.h
              TransientStreamer.h
         DESTINATION  ${product}/${version}/include/art/Framework/IO/Common )
