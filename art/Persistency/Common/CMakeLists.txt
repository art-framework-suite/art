
install(FILES Association.h
	      AssociationMap.h
	      AssociationMapHelpers.h
	      AssociationVector.h
	      AssociativeIterator.h
	      BaseHolder.h
	      BaseVectorHolder.h
	      BasicHandle.h
	      BoolCache.h
	      CacheStreamers.h
	      ClonePolicy.h
	      CloneTrait.h
	      CloningPtr.h
	      ConstPtrCache.h
	      CopyPolicy.h
	      DataFrameContainer.h
	      DataFrame.h
	      debugging_allocator.h
	      DetSet2RangeMap.h
	      DetSetAlgorithm.h
	      DetSet.h
	      DetSetLazyVector.h
	      DetSetNew.h
	      DetSetRefVector.h
	      DetSetVector.h
	      DetSetVectorNew.h
	      EDCollection.h
	      EDProductfwd.h
	      EDProductGetter.h
	      EDProduct.h
	      fillPtrVector.h
	      FillView.h
	      GetProduct.h
	      getRef.h
	      Handle.h
	      HLTenums.h
	      HLTGlobalStatus.h
	      HLTPathStatus.h
	      Holder.h
	      HolderToVectorTrait.h
	      IDVectorMap.h
	      IndirectHolder.h
	      IndirectVectorHolder.h
	      LazyGetter.h
	      MapOfVectors.h
	      MapRefViewTrait.h
	      NewPolicy.h
	      OneToMany.h
	      OneToManyWithQualityGeneric.h
	      OneToManyWithQuality.h
	      OneToOneGeneric.h
	      OneToOne.h
	      OneToValue.h
	      OrphanHandle.h
	      OutputHandle.h
	      OwnVector.h
	      PostReadFixupTrait.h
	      Ptr.h
	      PtrVectorBase.h
	      PtrVector.h
	      RangeMap.h
	      RefBase.h
	      RefCoreGet.h
	      RefCore.h
	      RefCoreStreamer.h
	      RefGetter.h
	      Ref.h
	      RefHolderBase.h
	      RefHolder_.h
	      RefHolder.h
	      RefItemGet.h
	      RefItem.h
	      RefKeyTrait.h
	      RefProd.h
	      RefToBase.h
	      RefToBaseProd.h
	      RefToBaseVector.h
	      RefToPtr.h
	      RefTraits.h
	      RefVectorBase.h
	      RefVector.h
	      RefVectorHolderBase.h
	      RefVectorHolder.h
	      RefVectorIterator.h
	      RefVectorTraits.h
	      setPtr.h
	      SortedCollection.h
	      TestHandle.h
	      traits.h
	      TransientDataFrame.h
	      Trie.h
	      TriggerResults.h
	      ValueMap.h
	      Vector.h
	      VectorHolder.h
	      View.h
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
			       DetSetVectorNew.cc
			       EDProduct.cc
			       EDProductGetter.cc
			       PtrVectorBase.cc
			       RefCore.cc
			       RefCoreStreamer.cc
			       traits.cc
			       View.cc )

link_libraries( FWCoreUtilities
                FWCoreServiceRegistry
		${Boost_THREAD_LIBRARY}
		${REFLEX}
		${MATRIX}
		${CORE} )

add_library(DataFormatsCommon SHARED ${DataFormatsCommon_sources} )

build_plugin( DataFormats Common )

install ( TARGETS DataFormatsCommon DESTINATION lib )
