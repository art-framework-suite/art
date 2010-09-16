
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

build_plugin( Persistency Common )

install ( TARGETS artPersistencyCommon DESTINATION lib )
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
              Trie.h
              TriggerResults.h
              ValueMap.h
              Wrapper.h
        DESTINATION include/art/Persistency/Common )
