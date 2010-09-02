# project name
project(artPersistencyProvenance)

set( artPersistencyProvenance_sources BranchChildren.cc
				   BranchDescription.cc
				   BranchEntryDescription.cc
				   BranchID.cc
				   BranchIDListHelper.cc
				   BranchKey.cc
				   BranchMapper.cc
				   BranchType.cc
				   EntryDescription.cc
				   EventAux.cc
				   EventAuxiliary.cc
				   EventEntryDescription.cc
				   EventEntryInfo.cc
				   EventID.cc
				   FileFormatVersion.cc
				   FileID.cc
				   FileIndex.cc
				   Hash.cc
				   History.cc
				   SubRunAux.cc
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
				   RunAux.cc
				   RunAuxiliary.cc
				   RunID.cc
				   RunSubRunEntryInfo.cc
				   Timestamp.cc
				   TransientStreamer.cc )


link_libraries( artUtilities
		${Boost_THREAD_LIBRARY}
		${REFLEX}
		${CORE} )

add_library(artPersistencyProvenance SHARED ${artPersistencyProvenance_sources} )

build_plugin( Persistency Provenance )

install ( TARGETS artPersistencyProvenance DESTINATION lib )
install(FILES BranchChildren.h
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
	      EventAux.h
	      EventAuxiliary.h
	      EventEntryDescription.h
	      EventEntryInfo.h
	      EventID.h
	      EventProcessHistoryID.h
	      EventSelectionID.h
	      FileFormatVersion.h
	      FileID.h
	      FileIndex.h
	      HashedTypes.h
	      Hash.h
	      History.h
	      SubRunAux.h
	      SubRunAuxiliary.h
	      SubRunID.h
	      ModuleDescription.h
	      ModuleDescriptionID.h
	      ModuleDescriptionRegistry.h
	      ParameterSetBlob.h
	      ParameterSetID.h
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
	      ProvenanceFwd.h
	      Provenance.h
	      ReleaseVersion.h
	      RunAux.h
	      RunAuxiliary.h
	      RunID.h
	      RunSubRunEntryInfo.h
	      Selections.h
	      Timestamp.h
	      Transient.h
	      TransientStreamer.h
	DESTINATION include/art/Persistency/Provenance )
