#ifndef art_Framework_Services_Registry_ServiceTable_h
#define art_Framework_Services_Registry_ServiceTable_h

#include "fhiclcpp/types/ConfigurationTable.h"
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
  class ServiceTable : public fhicl::ConfigurationTable {
  public:
    explicit ServiceTable(fhicl::Name&& name) : config_{std::move(name)} {}

    ServiceTable(fhicl::ParameterSet const& pset)
      : config_{fhicl::Name{"<service>"}}
    {
      std::set<std::string> const keys_to_ignore{"service_type",
                                                 "service_provider"};
      config_.validate_ParameterSet(pset, keys_to_ignore);
    }

    fhicl::ParameterSet const&
    get_PSet() const
    {
      return config_.get_PSet();
    }

    void
    print_allowed_configuration(std::ostream& os,
                                std::string const& prefix) const
    {
      config_.print_allowed_configuration(os, prefix);
    }

    auto const&
    operator()() const
    {
      return config_();
    }

  private:
    fhicl::Table<T> config_;
    cet::exempt_ptr<fhicl::detail::ParameterBase const>
    get_parameter_base() const override
    {
      return &config_;
    }
  };

  template <typename T>
  inline std::ostream&
  operator<<(std::ostream& os, ServiceTable<T> const& t)
  {
    std::ostringstream config;
    t.print_allowed_configuration(config, std::string(3, ' '));
    return os << config.str();
  }
}

#endif /* art_Framework_Services_Registry_ServiceTable_h */

// Local variables:
// mode: c++
// End:
