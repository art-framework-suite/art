#ifndef art_Framework_Art_detail_handle_deprecated_configs_h
#define art_Framework_Art_detail_handle_deprecated_configs_h

namespace fhicl {
  class intermediate_table;
}

namespace art {
  namespace detail {
    void handle_deprecated_configs(fhicl::intermediate_table&);

    void handle_deprecated_fileMode(fhicl::intermediate_table&);
    void handle_deprecated_SelectEvents(fhicl::intermediate_table&);
    void handle_deprecated_MemoryTracker(fhicl::intermediate_table&);
  }
}

#endif

// Local variables:
// mode: c++
// End:
