# project name
project(art_Persistency_Common)

set(art_Persistency_Common_sources
  CacheStreamers.cc
  DelayedReader.cc
  EDProduct.cc
  EDProductGetter.cc
  GroupQueryResult.cc
  PtrVectorBase.cc
  RNGsnapshot.cc
  RefCore.cc
  detail/setPtrVectorBaseStreamer.cc
  traits.cc
)

link_libraries(
  art_Persistency_Provenance
  art_Utilities
  ${Boost_THREAD_LIBRARY}
  )

add_library(art_Persistency_Common SHARED ${art_Persistency_Common_sources})

art_dictionary(DICTIONARY_LIBRARIES art_Persistency_Common)

install(TARGETS art_Persistency_Common DESTINATION ${flavorqual_dir}/lib)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
