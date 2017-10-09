#ifndef art_Framework_Core_ProducerBase_h
#define art_Framework_Core_ProducerBase_h
// vim: set sw=2 expandtab :

//
// The base class of all "modules" that will insert new
// EDProducts into an Event.
//

#include "art/Framework/Core/ModuleBase.h"
#include "art/Framework/Core/ModuleType.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Core/detail/IgnoreModuleLabel.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ProductSemantics.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/ConfigurationTable.h"
#include "fhiclcpp/types/KeysToIgnore.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TableFragment.h"

#include <functional>
#include <memory>
#include <set>
#include <sstream>
#include <string>

namespace art {

  class BranchDescription;

  class ProducerBase : public ModuleBase, private ProductRegistryHelper {

  public: // CONFIGURATION
    template <typename UserConfig, typename UserKeysToIgnore = void>
    class Table : public fhicl::ConfigurationTable {

    private: // TYPES
      template <typename T>
      struct FullConfig {

        fhicl::Atom<std::string> module_type{fhicl::Name("module_type")};

        fhicl::Atom<bool> errorOnFailureToPut{
          fhicl::Name("errorOnFailureToPut"),
          true};

        fhicl::TableFragment<T> user;
      };

      using KeysToIgnore_t = std::conditional_t<
        std::is_void<UserKeysToIgnore>::value,
        detail::IgnoreModuleLabel,
        fhicl::KeysToIgnore<detail::IgnoreModuleLabel, UserKeysToIgnore>>;

    public: // MEMBER FUNCTIONS -- Special Member Functions
      explicit Table(fhicl::Name&& name) : fullConfig_{std::move(name)} {}

      Table(fhicl::ParameterSet const& pset) : fullConfig_{pset} {}

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
      fhicl::Table<FullConfig<UserConfig>, KeysToIgnore_t> fullConfig_;
    };

  public: // MEMBER FUNCTIONS -- Special Member Functions
    virtual ~ProducerBase();

    ProducerBase();

    ProducerBase(ProducerBase const&) = delete;

    ProducerBase(ProducerBase&&) = delete;

    ProducerBase& operator=(ProducerBase const&) = delete;

    ProducerBase& operator=(ProducerBase&&) = delete;

  public: // MEMBER FUNCTIONS -- Product Registry Helper API
    using ProductRegistryHelper::expectedProducts;
    using ProductRegistryHelper::produces;
    using ProductRegistryHelper::registerProducts;
  };

  template <typename T>
  inline std::ostream&
  operator<<(std::ostream& os, ProducerBase::Table<T> const& t)
  {
    std::ostringstream config;
    t.print_allowed_configuration(config, std::string(3, ' '));
    return os << config.str();
  }

} // namespace art

#endif /* art_Framework_Core_ProducerBase_h */

// Local Variables:
// mode: c++
// End:
