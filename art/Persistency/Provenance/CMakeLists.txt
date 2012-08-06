# project name
project(art_Persistency_Provenance)

set(art_Persistency_Provenance_sources
  BranchChildren.cc
  BranchDescription.cc
  BranchID.cc
  BranchIDListHelper.cc
  BranchKey.cc
  BranchMapper.cc
  BranchType.cc
  EventAuxiliary.cc
  EventID.cc
  FileFormatVersion.cc
  FileIndex.cc
  Hash.cc
  History.cc
  SubRunAuxiliary.cc
  SubRunID.cc
  MasterProductRegistry.cc
  ModuleDescription.cc
  ParameterSetBlob.cc
  Parentage.cc
  ProcessConfiguration.cc
  ProcessHistory.cc
  ProductID.cc
  ProductMetaData.cc
  ProductProvenance.cc
  ReflexTools.cc
  RunAuxiliary.cc
  RunID.cc
  TransientStreamer.cc )


link_libraries( 
  art_Persistency_RootDB
  art_Utilities
  ${Boost_THREAD_LIBRARY}
  ${ROOT_CINTEX}
  )

add_library(art_Persistency_Provenance SHARED ${art_Persistency_Provenance_sources} )

art_dictionary(DICTIONARY_LIBRARIES art_Persistency_Provenance)

install ( TARGETS art_Persistency_Provenance DESTINATION ${flavorqual_dir}/lib )
install_headers()
install_source()
