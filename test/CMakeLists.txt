cet_set_compiler_flags(ENABLE_ASSERTS DIAGS VIGILANT EXTRA_FLAGS -pedantic)

# add_definitions(-DART_NO_MIX_PTRVECTOR)

# build Persistency libraries
subdirs (Framework/Art)
subdirs (Framework/Core)
subdirs (Framework/IO/Root)
subdirs (Framework/Principal)
subdirs (Integration)
subdirs (Persistency/Provenance)
subdirs (Persistency/RootDB)
subdirs (TestObjects)
subdirs (Version)
subdirs (Utilities)
