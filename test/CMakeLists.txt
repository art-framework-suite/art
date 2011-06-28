# project name
project(artTest)

# add_definitions(-DART_NO_MIX_PTRVECTOR)

# build Persistency libraries
subdirs (Framework/Core)
subdirs (Framework/IO/Root)
subdirs (Integration)
subdirs (Persistency/Provenance)
subdirs (TestObjects)
subdirs (Version)
subdirs (Utilities)
