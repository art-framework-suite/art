project(art_Framework_IO_Root)

set(art_Framework_IO_Root_sources
  DuplicateChecker.cc
  RootDelayedReader.cc
  RootInputFile.cc
  RootInputFileSequence.cc
  RootOutputFile.cc
  RootOutputTree.cc
  RootTree.cc
  )

add_library(art_Framework_IO_RootVersion SHARED
  GetFileFormatEra.cc
  GetFileFormatVersion.cc
)

add_library(art_Framework_IO_Root SHARED ${art_Framework_IO_Root_sources} )
target_link_libraries(art_Framework_IO_Root art_Framework_IO_RootVersion)

install(TARGETS art_Framework_IO_Root art_Framework_IO_RootVersion DESTINATION ${flavorqual_dir}/lib )

simple_plugin(RootInput "source" art_Framework_IO_Root art_Framework_IO_Sources )
simple_plugin(RootOutput "module" art_Framework_IO_Root)

install_source()
