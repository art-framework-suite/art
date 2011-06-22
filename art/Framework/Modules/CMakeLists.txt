# project name
project(art_Framework_Modules)

simple_plugin(EmptyEvent              "source")
simple_plugin(Prescaler               "module")
simple_plugin(ProvenanceCheckerOutput "module")
simple_plugin(FileDumperOutput        "module")

install_headers()
install_source()
