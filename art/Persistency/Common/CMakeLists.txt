
# project name
project(art_Persistency_Common)

set( art_Persistency_Common_sources CacheStreamers.cc
                                  EDProduct.cc
                                  EDProductGetter.cc
                                  InitRootHandlers.cc
#                                  PtrVectorBase.cc
                                  RNGsnapshot.cc
                                  RefCore.cc
                                  RefCoreStreamer.cc
                                  traits.cc)

#link_libraries( art_Utilities
#                art_Framework_Services_Registry
#               ${Boost_THREAD_LIBRARY}
#               ${REFLEX}
#               ${MATRIX}
#               ${CORE} )

add_library(art_Persistency_Common SHARED ${art_Persistency_Common_sources} )

build_dictionary( DICTIONARY_LIBRARIES art_Persistency_Common )

install ( TARGETS art_Persistency_Common DESTINATION ${flavorqual_dir}/lib )
install(FILES BaseHolder.h
	      BaseVectorHolder.h
	      BasicHandle.h
	      BoolCache.h
	      CacheStreamers.h
	      ClonePolicy.h
	      CloningPtr.h
	      ConstPtrCache.h
	      DetSet.h
	      DetSetNew.h
	      EDCollection.h
	      EDProduct.h
	      EDProductGetter.h
	      EDProductfwd.h
	      GetProduct.h
	      HLTGlobalStatus.h
	      HLTPathStatus.h
	      HLTenums.h
	      Handle.h
	      HolderToVectorTrait.h
	      IDVectorMap.h
	      MapOfVectors.h
	      NewPolicy.h
	      OrphanHandle.h
	      OutputHandle.h
	      PostReadFixupTrait.h
	      Ptr.h
	      PtrVector.h
	      PtrVectorBase.h
	      RNGsnapshot.h
	      RefCore.h
	      RefCoreStreamer.h
	      TestHandle.h
	      Trie.h
	      TriggerResults.h
	      Wrapper.h
	      debugging_allocator.h
	      traits.h
         DESTINATION  ${product}/${version}/include/art/Persistency/Common )
