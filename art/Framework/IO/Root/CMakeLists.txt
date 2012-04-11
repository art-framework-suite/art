project(art_Framework_IO_Root)

set(art_Framework_IO_Root_sources
  DuplicateChecker.cc
  FastCloningInfoProvider.cc
  RefCoreStreamer.cc
  RootBranchInfo.cc
  RootBranchInfoList.cc
  RootDelayedReader.cc
  RootInputFile.cc
  RootInputFileSequence.cc
  RootOutputFile.cc
  RootOutputTree.cc
  RootTree.cc
  rootNames.cc
  )

add_library(art_Framework_IO_RootVersion SHARED
  GetFileFormatEra.cc
  GetFileFormatVersion.cc
)

add_library(art_Framework_IO_Root SHARED ${art_Framework_IO_Root_sources} )
target_link_libraries(art_Framework_IO_Root
  art_Framework_Core
  art_Framework_IO_Catalog
  art_Framework_Principal
  art_Framework_Services_Registry
  art_Persistency_Common
  art_Persistency_Provenance
  art_Framework_IO_RootVersion)

install(TARGETS art_Framework_IO_Root art_Framework_IO_RootVersion DESTINATION ${flavorqual_dir}/lib )

simple_plugin(RootInput "source" art_Framework_IO_Root art_Framework_IO_Catalog )
simple_plugin(RootOutput "module" art_Framework_IO_Root)

add_executable( config_dumper config_dumper.cc )
target_link_libraries( config_dumper
        art_Framework_IO_Root
        art_Utilities
        art_Framework_Core
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
        ${ROOT_CINTEX}
        ${ROOT_PHYSICS}
        ${ROOTSYS}/lib/libGraf.so
        ${ROOT_TREE}
        ${ROOT_HIST}
        ${ROOT_MATRIX}
        ${ROOT_NET}
        ${ROOT_MATHCORE}
        ${ROOT_THREAD}
        ${ROOT_RIO}
        ${ROOT_CORE}
        ${ROOT_CINT}
        ${ROOT_REFLEX}
        -ldl
        )

install ( TARGETS config_dumper DESTINATION ${flavorqual_dir}/bin )

add_executable( sam_metadata_dumper sam_metadata_dumper.cc )
target_link_libraries( sam_metadata_dumper
        art_Framework_IO_Root
        art_Utilities
        art_Framework_Core
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
        ${ROOT_CINTEX}
        ${ROOT_PHYSICS}
        ${ROOTSYS}/lib/libGraf.so
        ${ROOT_TREE}
        ${ROOT_HIST}
        ${ROOT_MATRIX}
        ${ROOT_NET}
        ${ROOT_MATHCORE}
        ${ROOT_THREAD}
        ${ROOT_RIO}
        ${ROOT_CORE}
        ${ROOT_CINT}
        ${ROOT_REFLEX}
        -ldl
        )

install ( TARGETS sam_metadata_dumper DESTINATION ${flavorqual_dir}/bin )

install(FILES
  BranchMapperWithReader.h
  DuplicateChecker.h
  FastCloningInfoProvider.h
  GetFileFormatEra.h
  GetFileFormatVersion.h
  Inputfwd.h
  RefCoreStreamer.h
  RootBranchInfo.h
  RootBranchInfoList.h
  RootDelayedReader.h
  RootInput.h
  RootInputFile.h
  RootInputFileSequence.h
  RootOutput.h
  RootOutputFile.h
  RootOutputTree.h
  RootTree.h
  rootNames.h
  DESTINATION  ${product}/${version}/include/art/Framework/IO/Root
)

install_source()
