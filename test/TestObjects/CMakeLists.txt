project ( art_TestObjects )

install(FILES 
#OtherThingCollectionfwd.h
#	      OtherThingCollection.h
#	      OtherThing.h
#	      StreamTestSimple.h
#	      StreamTestThing.h
#	      StreamTestTmpl.h
#	      ThingCollectionfwd.h
#	      ThingCollection.h
#	      Thing.h
#	      ThingWithIsEqual.h
#	      ThingWithMerge.h
	      ToyProducts.h
	DESTINATION ${product}/${version}/include/art/TestObjects )

art_dictionary( DICTIONARY_LIBRARIES art_Persistency_Common_dict NOINSTALL )
