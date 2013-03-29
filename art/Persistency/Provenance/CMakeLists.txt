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
  ReflexTools.cc
  RunAuxiliary.cc
  SubRunAuxiliary.cc
  TransientStreamer.cc
 )


set( art_Persistency_Provenance_libraries
  art_Persistency_RootDB
  art_Utilities
  ${Boost_THREAD_LIBRARY}
  ${ROOT_CINTEX}
  )

art_make_library( LIBRARY_NAME art_Persistency_Provenance
                  SOURCE ${art_Persistency_Provenance_sources}
                  LIBRARIES ${art_Persistency_Provenance_libraries}
		 )

art_dictionary(DICTIONARY_LIBRARIES art_Persistency_Provenance)

install_headers()
install_source()
