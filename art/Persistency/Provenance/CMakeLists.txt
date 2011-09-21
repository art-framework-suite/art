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
  )

add_library(art_Persistency_Provenance SHARED ${art_Persistency_Provenance_sources} )

art_dictionary(DICTIONARY_LIBRARIES art_Persistency_Provenance)

install ( TARGETS art_Persistency_Provenance DESTINATION ${flavorqual_dir}/lib )
install(FILES
  BranchChildren.h
  BranchDescription.h
  BranchID.h
  BranchIDList.h
  BranchIDListHelper.h
  BranchIDListRegistry.h
  BranchKey.h
  BranchListIndex.h
  BranchMapper.h
  BranchType.h
  EventAuxiliary.h
  EventID.h
  EventProcessHistoryID.h
  EventSelectionID.h
  FileFormatVersion.h
  FileIndex.h
  Hash.h
  HashedTypes.h
  History.h
  MasterProductRegistry.h
  ModuleDescription.h
  ModuleDescriptionID.h
  ParameterSetBlob.h
  ParameterSetMap.h
  Parentage.h
  ParentageID.h
  ParentageRegistry.h
  PassID.h
  ProcessConfiguration.h
  ProcessConfigurationID.h
  ProcessConfigurationRegistry.h
  ProcessHistory.h
  ProcessHistoryID.h
  ProcessHistoryRegistry.h
  ProductID.h
  ProductMetaData.h
  ProductProvenance.h
  ProductRegistry.h
  ProductStatus.h
  ProvenanceFwd.h
  ReflexTools.h
  ReleaseVersion.h
  RunAuxiliary.h
  RunID.h
  Selections.h
  SortInvalidFirst.h
  SubRunAuxiliary.h
  SubRunID.h
  Timestamp.h
  Transient.h
  TransientStreamer.h
  TypeLabel.h
  DESTINATION  ${product}/${version}/include/art/Persistency/Provenance )

install_source()
