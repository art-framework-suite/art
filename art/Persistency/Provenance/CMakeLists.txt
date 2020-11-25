art_make_library(
  LIBRARY_NAME art_Persistency_Provenance
  SOURCE
    detail/branchNameComponentChecking.cc
    ModuleDescription.cc
    orderedProcessNamesCollection.cc
  LIBRARIES
    art::art_Utilities
  PRIVATE
    Range::Range
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
