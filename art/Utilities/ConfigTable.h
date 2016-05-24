#ifndef art_Utilities_ConfigTable_h
#define art_Utilities_ConfigTable_h

#include "fhiclcpp/types/Table.h"

#include <sstream>

namespace art {

  template <typename DETAIL, typename F>
  class ConfigTable {
  public:

    ConfigTable() = default;
    ConfigTable(fhicl::ParameterSet const& pset)
    {
      config_.validate_ParameterSet(pset, F{}());
    }

    fhicl::ParameterSet const& get_PSet() const
    {
      return config_.get_PSet();
    }

    auto const & operator()() const { return config_(); }

    void print_allowed_configuration(std::ostream& os, std::string const& prefix) const
    {
      config_.print_allowed_configuration(os, prefix);
    }

  private:
    fhicl::Table<DETAIL> config_ { fhicl::Name("<config>") };

  };

  template <typename T, typename DETAIL, typename F>
  inline decltype(auto) operator<<(T&& t, ConfigTable<DETAIL, F> const& u)
  {
    std::ostringstream oss;
    u.print_allowed_configuration(oss, std::string(3,' '));
    return std::forward<T>(t) << oss.str();
  }

}

#endif /* art_Utilities_ConfigTable_h */

// Local variables:
// mode: c++
// End:
