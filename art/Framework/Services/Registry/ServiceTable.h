#ifndef art_Framework_Services_Registry_ServiceTable_h
#define art_Framework_Services_Registry_ServiceTable_h

#include "fhiclcpp/Table.h"

#include <set>
#include <string>

namespace fhicl {
  class ParameterSet;
}

namespace art {

  template <typename T>
  class ServiceTable : public fhicl::Table<T> {
  public:

    ServiceTable() {}

    ServiceTable( fhicl::ParameterSet const & pset ) : ServiceTable()
    {
      std::set<std::string> const keys_to_ignore = { "service_type", "service_provider"};
      this->validate_ParameterSet( pset, keys_to_ignore );
      this->set_PSet( pset );
    }

  };

}

#endif

// Local variables:
// mode: c++
// End:
