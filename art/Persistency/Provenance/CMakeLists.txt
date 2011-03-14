# project name
project(art_Persistency_Provenance)

set( art_Persistency_Provenance_sources BranchChildren.cc
                                      BranchDescription.cc
                                      BranchEntryDescription.cc
                                      BranchID.cc
                                      BranchIDListHelper.cc
                                      BranchKey.cc
                                      BranchMapper.cc
                                      BranchType.cc
                                      EntryDescription.cc
                                      EventAuxiliary.cc
                                      EventEntryDescription.cc
                                      EventEntryInfo.cc
                                      EventID.cc
                                      FileFormatVersion.cc
                                      FileID.cc
                                      FileIndex.cc
                                      Hash.cc
                                      History.cc
                                      SubRunAuxiliary.cc
                                      SubRunID.cc
                                      ModuleDescription.cc
                                      ParameterSetBlob.cc
                                      Parentage.cc
                                      ProcessConfiguration.cc
                                      ProcessHistory.cc
                                      ProductID.cc
                                      ProductProvenance.cc
                                      ProductRegistry.cc
                                      Provenance.cc
                                      RunAuxiliary.cc
                                      RunID.cc
                                      RunSubRunEntryInfo.cc
                                      TransientStreamer.cc )


#link_libraries( art_Utilities
#               ${Boost_THREAD_LIBRARY}
#               ${REFLEX}
#               ${CORE} )

add_library(art_Persistency_Provenance SHARED ${art_Persistency_Provenance_sources} )

build_dictionary( DICTIONARY_LIBRARIES art_Persistency_Provenance )

install ( TARGETS art_Persistency_Provenance DESTINATION ${flavorqual_dir}/lib )
install(FILES
  BranchChildren.h
  BranchDescription.h
  BranchEntryDescription.h
  BranchID.h
  BranchIDList.h
  BranchIDListHelper.h
  BranchIDListRegistry.h
  BranchKey.h
  BranchListIndex.h
  BranchMapper.h
  BranchType.h
  ConstBranchDescription.h
  EntryDescription.h
  EntryDescriptionID.h
  EntryDescriptionRegistry.h
  EventAuxiliary.h
  EventEntryDescription.h
  EventEntryInfo.h
  EventID.h
  EventProcessHistoryID.h
  EventSelectionID.h
  FileFormatVersion.h
  FileID.h
  FileIndex.h
  Hash.h
  HashedTypes.h
  History.h
  ModuleDescription.h
  ModuleDescriptionID.h
  ModuleDescriptionRegistry.h
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
  ProductProvenance.h
  ProductRegistry.h
  ProductStatus.h
  Provenance.h
  ProvenanceFwd.h
  ReleaseVersion.h
  RunAuxiliary.h
  RunID.h
  RunSubRunEntryInfo.h
  Selections.h
  SortInvalidFirst.h
  SubRunAuxiliary.h
  SubRunID.h
  Timestamp.h
  Transient.h
  TransientStreamer.h
  DESTINATION  ${product}/${version}/include/art/Persistency/Provenance )

install_source()
