# project name
project(art_Persistency_Common)

set(art_Persistency_Common_sources
  CacheStreamers.cc
  EDProduct.cc
  EDProductGetter.cc
  InitRootHandlers.cc
  PtrVectorBase.cc
  RNGsnapshot.cc
  RefCore.cc
  RefCoreTransientStreamer.cc
  detail/setPtrVectorBaseStreamer.cc
  traits.cc
)

link_libraries(art_Utilities
               art_Framework_Services_Registry
               ${Boost_THREAD_LIBRARY}
)

add_library(art_Persistency_Common SHARED ${art_Persistency_Common_sources})

art_dictionary(DICTIONARY_LIBRARIES art_Persistency_Common)

install(TARGETS art_Persistency_Common DESTINATION ${flavorqual_dir}/lib)
install(FILES
  Assns.h
  BoolCache.h
  CacheStreamers.h
  CollectionUtilities.h
  ConstPtrCache.h
  EDProduct.h
  EDProductGetter.h
  GetProduct.h
  HLTGlobalStatus.h
  HLTPathStatus.h
  HLTenums.h
  Handle.h
  OrphanHandle.h
  OutputHandle.h
  PostReadFixupTrait.h
  Ptr.h
  PtrVector.h
  PtrVectorBase.h
  RNGsnapshot.h
  RefCore.h
  RefCoreTransientStreamer.h
  TriggerResults.h
  Wrapper.h
  debugging_allocator.h
  detail/setPtrVectorBaseStreamer.h
  fwd.h
  traits.h
  InitRootHandlers.h
  DESTINATION  ${product}/${version}/include/art/Persistency/Common
)

install_source()
