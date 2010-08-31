# project name
project(DataFormats)

# version number
set (DataFormats_VERSION_MAJOR 1)
set (DataFormats_VERSION_MINOR 0)

# build DataFormats libraries
subdirs (Common/src)
subdirs (Provenance/src)
subdirs (StdDictionaries/src)
subdirs (TestObjects/src)
subdirs (WrappedStdDictionaries/src)

# build DataFormats tests 
subdirs (Common/test)
subdirs (Provenance/test)

# directories that have install instructions, but nothing else
subdirs (Common/interface)
subdirs (Provenance/interface)
subdirs (TestObjects/interface)
