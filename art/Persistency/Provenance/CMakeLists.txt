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
  MasterProductRegistry.cc
  ModuleDescription.cc
  ParameterSetBlob.cc
  Parentage.cc
  ProcessConfiguration.cc
  ProcessHistory.cc
  ProductID.cc
  ProductMetaData.cc
  ProductProvenance.cc
  RunAuxiliary.cc
  RunID.cc
  SubRunAuxiliary.cc
  SubRunID.cc
  TransientStreamer.cc
  TypeTools.cc
  TypeWithDict.cc
 )


set( art_Persistency_Provenance_libraries
  art_Persistency_RootDB
  art_Utilities
  MF_MessageLogger
  ${ROOT_CORE}
  ${Boost_THREAD_LIBRARY}
  )

art_make_library( LIBRARY_NAME art_Persistency_Provenance
                  SOURCE ${art_Persistency_Provenance_sources}
                  LIBRARIES ${art_Persistency_Provenance_libraries}
		 )

art_dictionary(DICTIONARY_LIBRARIES art_Persistency_Provenance REQUIRED_DICTIONARIES art_Persistency_FhiclCppDictionaries_dict)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
