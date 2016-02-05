# project name
project(art_Persistency_Common)

set(art_Persistency_Common_sources
  DelayedReader.cc
  GroupQueryResult.cc
)

set( art_Persistency_Common_libraries 
	canvas_Persistency_Common
	art_Persistency_Provenance 
	canvas_Persistency_Provenance
	art_Utilities 
	canvas_Utilities
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
