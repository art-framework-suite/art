add_subdirectory(RootDB)
add_subdirectory(Services)

art_make_library(
  LIBRARY_NAME art_Framework_IO_Root_detail_sources
  SOURCE
  detail/DummyProductCache.cc
  detail/RangeSetInfo.cc
  detail/getEntry.cc
  detail/getObjectRequireDict.cc
  detail/rangeSetFromFileIndex.cc
  detail/resolveRangeSet.cc
  detail/rootFileSizeTools.cc
  detail/rootOutputConfigurationTools.cc
  LIBRARIES
  art_Framework_Core
  art_Framework_Principal
  art_Framework_IO_Root_RootDB
  art_Utilities
  canvas
  canvas_root_io
  hep_concurrency
  MF_MessageLogger
  ${ROOT_CORE}
  ${ROOT_RIO}
  ${ROOT_TREE})

art_make_library(
  LIBRARY_NAME art_Framework_IO_RootVersion
  SOURCE GetFileFormatEra.cc GetFileFormatVersion.cc)

art_make_library(
  LIBRARY_NAME art_Framework_IO_Root_file_info_dumper
  SOURCE
  detail/InfoDumperInputFile.cc
  LIBRARIES
  art_Framework_IO_Root_detail_sources
  art_Framework_IO_Root_RootDB
  canvas
  cetlib)

art_make(EXCLUDE
  GetFileFormatEra.cc
  GetFileFormatVersion.cc
  config_dumper.cc
  count_events.cc
  file_info_dumper.cc
  product_sizes_dumper.cc
  sam_metadata_dumper.cc
  RootInputTree.cc
  NO_PLUGINS
  LIB_LIBRARIES
  art_Framework_Core
  art_Framework_IO
  art_Framework_IO_Catalog
  art_Framework_IO_detail
  art_Framework_Services_Registry
  art_Framework_Services_System_FileCatalogMetadata_service
  art_Persistency_Common
  art_Persistency_Provenance
  art_Framework_IO_RootVersion
  art_Framework_IO_Root_detail_sources
  cetlib
  canvas
  ${ROOT_HIST}
  ${ROOT_TREE})

simple_plugin(RootInput "source" art_Framework_IO_Root art_Framework_IO_Catalog)
simple_plugin(RootOutput "module" art_Framework_IO_Root)

art_make_exec(NAME product_sizes_dumper
  LIBRARIES
  art_Framework_IO_Root
  cetlib
  ${Boost_PROGRAM_OPTIONS_LIBRARY})

art_make_exec(NAME config_dumper
  LIBRARIES art_Framework_IO_Root
  art_Utilities canvas
  art_Framework_Core
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${ROOT_TREE}
  ${ROOT_RIO})

art_make_exec(NAME sam_metadata_dumper
  LIBRARIES art_Framework_IO_Root
  art_Utilities canvas
  art_Framework_Core
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${ROOT_RIO})

art_make_exec(NAME count_events
  LIBRARIES
  art_Framework_IO_Root
  art_Framework_Core
  art_Utilities canvas
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${ROOT_TREE}
  ${ROOT_RIO})

art_make_exec(NAME file_info_dumper
  LIBRARIES art_Framework_IO_Root
  art_Framework_IO_Root_file_info_dumper
  art_Framework_Principal
  ${Boost_PROGRAM_OPTIONS_LIBRARY})

cet_make_completions(product_sizes_dumper)
cet_make_completions(config_dumper)
cet_make_completions(sam_metadata_dumper)
cet_make_completions(count_events)
cet_make_completions(file_info_dumper)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
