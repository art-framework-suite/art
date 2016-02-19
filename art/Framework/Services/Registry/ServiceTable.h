#ifndef art_Framework_Services_Registry_ServiceTable_h
#define art_Framework_Services_Registry_ServiceTable_h

#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/detail/validationException.h"

#include <ostream>
#include <set>
#include <string>

namespace fhicl {
  class ParameterSet;
}

namespace art {

  template <typename T>
  class ServiceTable {
  public:

    ServiceTable() = default;

    ServiceTable(fhicl::ParameterSet const & pset) : ServiceTable()
    {
      std::set<std::string> const keys_to_ignore = {"service_type", "service_provider"};
      try {
        config_.validate_ParameterSet(pset, keys_to_ignore);
      }
      catch (fhicl::detail::validationException const&) {
        throw;
      }
    }

    fhicl::ParameterSet const& get_PSet() const { return config_.get_PSet(); }

    void print_allowed_configuration(std::ostream& os, std::string const& prefix) const
    {
      config_.print_allowed_configuration(os, prefix);
    }

    auto const& operator()() const { return config_(); }

  private:
    fhicl::Table<T> config_ { fhicl::Name("<service>") };
  };

}

#endif /* art_Framework_Services_Registry_ServiceTable_h */

// Local variables:
// mode: c++
// End:
