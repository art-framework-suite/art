#ifndef art_Framework_Art_prune_physics_configuration_h
#define art_Framework_Art_prune_physics_configuration_h

namespace fhicl {
  class intermediate_table;
}

namespace art {
  namespace detail {
    void
    prune_configuration(fhicl::intermediate_table& config);
  }
}

#endif
