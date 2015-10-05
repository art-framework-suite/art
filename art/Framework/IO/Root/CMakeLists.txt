art_make_library(
  LIBRARY_NAME art_Framework_IO_Root_detail_sources
  SOURCE detail/rootFileSizeTools.cc
  LIBRARIES
  ${ROOT_CORE}
  ${ROOT_RIO}
  ${ROOT_TREE}
  )

art_make_library(
  LIBRARY_NAME art_Framework_IO_RootVersion
  SOURCE GetFileFormatEra.cc GetFileFormatVersion.cc
  )

art_make(EXCLUDE
  GetFileFormatEra.cc
  GetFileFormatVersion.cc
  config_dumper.cc
  count_events.cc
  product_sizes_dumper.cc
  sam_metadata_dumper.cc
  NO_PLUGINS
  LIB_LIBRARIES 
  art_Framework_Core
  art_Framework_IO
  art_Framework_IO_Catalog
  art_Framework_Principal
  art_Framework_Services_Registry
  art_Persistency_Common
  art_Persistency_Provenance
  art_Framework_IO_RootVersion
  art_Framework_IO_Root_detail_sources
  ${ROOT_CINTEX}
  ${ROOT_TREE}
  ${ROOT_NET}
  ${ROOT_MATHCORE}
)

simple_plugin(RootInput "source" art_Framework_IO_Root art_Framework_IO_Catalog )
simple_plugin(RootOutput "module" art_Framework_IO_Root)

art_make_exec( NAME product_sizes_dumper
  LIBRARIES
  art_Framework_IO_Root
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${CETLIB}
  )

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

art_make_exec( NAME count_events
  LIBRARIES
  art_Framework_IO_Root
  art_Framework_Core
  art_Utilities
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${ROOT_TREE}
  ${ROOT_RIO}
  )

install_headers()
install_source()
