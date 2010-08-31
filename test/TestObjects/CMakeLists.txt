
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
	DESTINATION include/DataFormats/TestObjects/interface )
>>><<<
# project name
project(DataFormatsTestObjects)

# version number
set (DataFormatsTestObjects_VERSION_MAJOR 1)
set (DataFormatsTestObjects_VERSION_MINOR 0)

build_plugin( DataFormats TestObjects )
