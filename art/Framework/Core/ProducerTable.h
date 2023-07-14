#ifndef art_Framework_Core_ProducerTable_h
#define art_Framework_Core_ProducerTable_h
// vim: set sw=2 expandtab :

#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/ConfigurationTable.h"
#include "fhiclcpp/types/KeysToIgnore.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TableFragment.h"

#include <sstream>
#include <string>

namespace art {
  template <typename UserConfig,
            typename ImplicitConfig,
            typename... UserKeysToIgnore>
  class ProducerTable : public fhicl::ConfigurationTable {
  private: // TYPES
    template <typename T, typename U = ImplicitConfig>
    struct FullConfig {
      fhicl::Atom<std::string> type{U::plugin_type()};
      fhicl::Atom<bool> errorOnFailureToPut{fhicl::Name("errorOnFailureToPut"),
                                            true};
      fhicl::TableFragment<T> user;
    };

    using KeysToIgnore_t = fhicl::KeysToIgnore<typename ImplicitConfig::IgnoreKeys, UserKeysToIgnore...>;

  public: // MEMBER FUNCTIONS -- Special Member Functions
    explicit ProducerTable(fhicl::Name&& name) : fullConfig_{std::move(name)} {}
    ProducerTable(fhicl::ParameterSet const& pset) : fullConfig_{pset} {}

  public: // MEMBER FUNCTIONS -- User-facing API
    auto const&
    operator()() const
    {
      return fullConfig_().user();
    }

    auto const&
    get_PSet() const
    {
      return fullConfig_.get_PSet();
    }

    void
    print_allowed_configuration(std::ostream& os,
                                std::string const& prefix) const
    {
      fullConfig_.print_allowed_configuration(os, prefix);
    }

  private: // MEMBER FUNCTIONS
    cet::exempt_ptr<fhicl::detail::ParameterBase const>
    get_parameter_base() const override
    {
      return &fullConfig_;
    }

  private: // DATA MEMBERS
    fhicl::Table<FullConfig<UserConfig, ImplicitConfig>, KeysToIgnore_t>
      fullConfig_;
  };

  template <typename UserConfig, typename ImplicitConfig>
  inline std::ostream&
  operator<<(std::ostream& os,
             ProducerTable<UserConfig, ImplicitConfig> const& t)
  {
    std::ostringstream config;
    t.print_allowed_configuration(config, std::string(3, ' '));
    return os << config.str();
  }
}

#endif /* art_Framework_Core_ProducerTable_h */

// Local Variables:
// mode: c++
// End:
