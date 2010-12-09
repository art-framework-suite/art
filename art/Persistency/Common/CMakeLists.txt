
# project name
project(artPersistencyCommon)

set( artPersistencyCommon_sources CacheStreamers.cc
                                  EDProduct.cc
                                  EDProductGetter.cc
                                  PtrVectorBase.cc
                                  RNGsnapshot.cc
                                  RefCore.cc
                                  RefCoreStreamer.cc
                                  traits.cc)

#link_libraries( artUtilities
#                artFrameworkServicesRegistry
#               ${Boost_THREAD_LIBRARY}
#               ${REFLEX}
#               ${MATRIX}
#               ${CORE} )

add_library(artPersistencyCommon SHARED ${artPersistencyCommon_sources} )

build_dictionary( DICTIONARY_LIBRARIES artPersistencyCommon )

install ( TARGETS artPersistencyCommon DESTINATION ${flavorqual_dir}/lib )
install(FILES AssociativeIterator.h
	      BaseHolder.h
	      BaseVectorHolder.h
	      BasicHandle.h
	      BoolCache.h
	      CacheStreamers.h
	      ClonePolicy.h
	      CloningPtr.h
	      ConstPtrCache.h
	      CopyPolicy.h
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
	      classes.h
	      debugging_allocator.h
	      traits.h
         DESTINATION  ${product}/${version}/include/art/Persistency/Common )
