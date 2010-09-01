
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
	      DataFrameContainer.h
	      DataFrame.h
	      debugging_allocator.h
	      DetSet.h
	      DetSetNew.h
	      EDCollection.h
	      EDProductfwd.h
	      EDProductGetter.h
	      EDProduct.h
	      GetProduct.h
	      getRef.h
	      Handle.h
	      HLTenums.h
	      HLTGlobalStatus.h
	      HLTPathStatus.h
	      HolderToVectorTrait.h
	      IDVectorMap.h
	      MapOfVectors.h
	      NewPolicy.h
	      OrphanHandle.h
	      OutputHandle.h
	      PostReadFixupTrait.h
	      Ptr.h
	      PtrVectorBase.h
	      PtrVector.h
              RNGsnapshot.h
	      RefCore.h
	      RefCoreStreamer.h
	      TestHandle.h
	      traits.h
	      TransientDataFrame.h
	      Trie.h
	      TriggerResults.h
	      ValueMap.h
	      Wrapper.h
	DESTINATION include/DataFormats/Common/interface )
>>><<<
# project name
project(DataFormatsCommon)

# version number
set (DataFormatsCommon_VERSION_MAJOR 1)
set (DataFormatsCommon_VERSION_MINOR 0)

set( DataFormatsCommon_sources CacheStreamers.cc
			       DataFrameContainer.cc
			       EDProduct.cc
			       EDProductGetter.cc
			       PtrVectorBase.cc
                               RNGsnapshot.cc
			       RefCore.cc
			       RefCoreStreamer.cc
			       traits.cc)

link_libraries( FWCoreUtilities
                FWCoreServiceRegistry
		${Boost_THREAD_LIBRARY}
		${REFLEX}
		${MATRIX}
		${CORE} )

add_library(DataFormatsCommon SHARED ${DataFormatsCommon_sources} )

build_plugin( DataFormats Common )

install ( TARGETS DataFormatsCommon DESTINATION lib )
