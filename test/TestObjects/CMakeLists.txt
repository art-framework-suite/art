
install(FILES OtherThingCollectionfwd.h
	      OtherThingCollection.h
	      OtherThing.h
	      StreamTestSimple.h
	      StreamTestThing.h
	      StreamTestTmpl.h
	      ThingCollectionfwd.h
	      ThingCollection.h
	      Thing.h
	      ThingWithIsEqual.h
	      ThingWithMerge.h
	      ToyProducts.h
	DESTINATION ${product}/${version}/include/DataFormats/TestObjects/interface )

build_dictionary( DataFormats TestObjects )
