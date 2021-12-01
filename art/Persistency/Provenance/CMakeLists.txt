cet_make_library(SOURCE
    detail/branchNameComponentChecking.cc
    ModuleDescription.cc
    PathSpec.cc
    orderedProcessNamesCollection.cc
  LIBRARIES
  PUBLIC
    art::Utilities
    canvas::canvas
    cetlib::container_algorithms
    fhiclcpp::fhiclcpp
  PRIVATE
    Boost::headers
    range-v3::range-v3
)

install_headers(SUBDIRS detail)
install_source(SUBDIRS detail)
