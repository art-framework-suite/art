project ( art_TestObjects )

install(FILES
	      ToyProducts.h
	DESTINATION ${product}/${version}/include/art/TestObjects )

art_dictionary( DICTIONARY_LIBRARIES art_Persistency_Common_dict NOINSTALL )
