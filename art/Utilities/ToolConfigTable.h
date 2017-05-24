#ifndef art_Utilities_ToolConfigTable_h
#define art_Utilities_ToolConfigTable_h

#include "art/Utilities/ConfigurationTable.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/KeysToIgnore.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TableFragment.h"

#include <set>
#include <string>
#include <type_traits>

namespace art {

  struct MinimalToolConfig {
    fhicl::Atom<std::string> tool_type { fhicl::Name("tool_type") };
    struct KeysToIgnore {
      std::set<std::string> operator()() { return {}; }
    };
  };

  template <typename UserConfig, typename UserKeysToIgnore = void>
  class ToolConfigTable : public ConfigurationTable {
  public:

    ToolConfigTable(fhicl::Name&& name) : fullConfig_{std::move(name)} {}
    ToolConfigTable(fhicl::ParameterSet const& pset) : fullConfig_{pset} {}

    auto const& operator()() const { return fullConfig_().user(); }
    auto const& get_PSet() const { return fullConfig_.get_PSet(); }

    void print_allowed_configuration(std::ostream& os, std::string const& prefix) const
    {
      fullConfig_.print_allowed_configuration(os, prefix);
    }

  private:

    template <typename T>
    struct FullConfig {
      fhicl::TableFragment<MinimalToolConfig> tool_type;
      fhicl::TableFragment<T> user;
    };

    using KeysToIgnore_t = std::conditional_t<std::is_void<UserKeysToIgnore>::value,
                                              MinimalToolConfig::KeysToIgnore,
                                              fhicl::KeysToIgnore<MinimalToolConfig::KeysToIgnore, UserKeysToIgnore>>;

    fhicl::Table<FullConfig<UserConfig>, KeysToIgnore_t> fullConfig_;
    cet::exempt_ptr<fhicl::detail::ParameterBase const> get_parameter_base() const override { return &fullConfig_; }
  };
}


#endif /* art_Utilities_ToolConfigTable_h */

// Local variables:
// mode: c++
// End:
