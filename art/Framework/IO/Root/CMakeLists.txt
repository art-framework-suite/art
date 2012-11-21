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

art_make_library( LIBRARY_NAME art_Framework_IO_RootVersion
                  SOURCE GetFileFormatEra.cc GetFileFormatVersion.cc
)

art_make_library( LIBRARY_NAME art_Framework_IO_Root
                  SOURCE ${art_Framework_IO_Root_sources}
                  LIBRARIES art_Framework_Core
  art_Framework_IO_Catalog
  art_Framework_Principal
  art_Framework_Services_Registry
  art_Persistency_Common
  art_Persistency_Provenance
  art_Framework_IO_RootVersion
  ${ROOT_TREE}
  )

simple_plugin(RootInput "source" art_Framework_IO_Root art_Framework_IO_Catalog )
simple_plugin(RootOutput "module" art_Framework_IO_Root)

art_make_exec( NAME config_dumper
               LIBRARIES art_Framework_IO_Root
        art_Utilities
        art_Framework_Core
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
        ${ROOT_TREE}
        ${ROOT_RIO}
        )

art_make_exec( NAME sam_metadata_dumper
               LIBRARIES art_Framework_IO_Root
        art_Utilities
        art_Framework_Core
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
        ${ROOT_RIO}
        )

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
