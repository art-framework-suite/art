
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
	      LuminosityBlockAux.h
	      LuminosityBlockAuxiliary.h
	      LuminosityBlockID.h
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
	      RNGsnapshot.h
	      RunAux.h
	      RunAuxiliary.h
	      RunID.h
	      RunLumiEntryInfo.h
	      Selections.h
	      Timestamp.h
	      Transient.h
	      TransientStreamer.h
	DESTINATION include/DataFormats/Provenance/interface )
>>><<<
# project name
project(DataFormatsProvenance)

# version number
set (DataFormatsProvenance_VERSION_MAJOR 1)
set (DataFormatsProvenance_VERSION_MINOR 0)

set( DataFormatsProvenance_sources BranchChildren.cc
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
				   LuminosityBlockAux.cc
				   LuminosityBlockAuxiliary.cc
				   LuminosityBlockID.cc
				   ModuleDescription.cc
				   ParameterSetBlob.cc
				   Parentage.cc
				   ProcessConfiguration.cc
				   ProcessHistory.cc
				   ProductID.cc
				   ProductProvenance.cc
				   ProductRegistry.cc
				   Provenance.cc
				   RNGsnapshot.cc
				   RunAux.cc
				   RunAuxiliary.cc
				   RunID.cc
				   RunLumiEntryInfo.cc
				   SConscript
				   Timestamp.cc
				   TransientStreamer.cc )


link_libraries( FWCoreUtilities
		${Boost_THREAD_LIBRARY}
		${REFLEX} 
		${CORE} )

add_library(DataFormatsProvenance SHARED ${DataFormatsProvenance_sources} )

build_plugin( DataFormats Provenance )

install ( TARGETS DataFormatsProvenance DESTINATION lib )
