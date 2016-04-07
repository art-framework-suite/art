#ifndef art_Framework_Art_detail_exists_outside_prolog_h
#define art_Framework_Art_detail_exists_outside_prolog_h

#include "fhiclcpp/intermediate_table.h"

namespace art {
  namespace detail {

    inline bool exists_outside_prolog (fhicl::intermediate_table const& config,
                                       std::string const& key)
    {
      return config.exists(key) && !config.find(key).in_prolog;
    }

  }
}

#endif /* art_Framework_Art_detail_exists_outside_prolog_h */

// Local variables:
// mode: c++
// End:
