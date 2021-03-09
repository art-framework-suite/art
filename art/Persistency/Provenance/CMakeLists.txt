cet_make_library(SOURCE
    detail/branchNameComponentChecking.cc
    ModuleDescription.cc
    orderedProcessNamesCollection.cc
  LIBRARIES
  PUBLIC
    art::Utilities
    canvas::canvas
    cetlib::container_algorithms
    fhiclcpp::fhiclcpp
  PRIVATE
    Boost::headers
    Range::Range
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
