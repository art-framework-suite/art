# project name
project(art_Framework_Modules)

simple_plugin(BlockingPrescaler       "module")
simple_plugin(EmptyEvent              "source")
simple_plugin(FileDumperOutput        "module")
simple_plugin(Prescaler               "module")
simple_plugin(ProvenanceCheckerOutput "module")
simple_plugin(RandomNumberSaver       "module")

art_dictionary(DICTIONARY_LIBRARIES art_Persistency_Common NO_CHECK_CLASS_VERSION)

install_headers()
install_source()
