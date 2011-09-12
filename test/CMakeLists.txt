# project name
project(artTest)

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
