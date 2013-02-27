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
  detail/maybeCastObj.cc
  detail/setPtrVectorBaseStreamer.cc
  traits.cc
)

set( art_Persistency_Common_libraries 
	art_Persistency_Provenance
	art_Utilities
	${ROOT_CORE}
	${Boost_THREAD_LIBRARY}
	)

art_make_library( LIBRARY_NAME art_Persistency_Common
                  SOURCE ${art_Persistency_Common_sources}
		  LIBRARIES ${art_Persistency_Common_libraries}
		 )

art_dictionary(DICTIONARY_LIBRARIES art_Persistency_Common)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
