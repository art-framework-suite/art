#ifndef art_Framework_Services_Registry_detail_ensure_only_on_thread_h
#define art_Framework_Services_Registry_detail_ensure_only_on_thread_h

namespace fhicl {
  class ParameterSet;
}

namespace art::detail {
  void ensure_only_one_thread(fhicl::ParameterSet const& service_pset);
}

#endif
