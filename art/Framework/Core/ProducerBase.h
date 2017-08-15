#ifndef art_Framework_Core_ProducerBase_h
#define art_Framework_Core_ProducerBase_h

//----------------------------------------------------------------------
// ProducerBase: The base class of all "modules" that will insert new
//               EDProducts into an Event.
//----------------------------------------------------------------------

#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Core/detail/IgnoreModuleLabel.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Principal/get_ProductDescription.h"
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
#include <string>
#include <sstream>

namespace art {

  class BranchDescription;
  class ModuleDescription;

  class ProducerBase : private ProductRegistryHelper {
  public:

    using ProductRegistryHelper::registerProducts;
    using ProductRegistryHelper::produces;
    using ProductRegistryHelper::expectedProducts;
    using ProductRegistryHelper::productLookups;
    using ProductRegistryHelper::elementLookups;

    bool modifiesEvent() const { return true; }

    template <typename PROD, BranchType B>
    ProductID getProductID(ModuleDescription const& moduleDescription,
                           std::string const& instanceName) const;

    // Configuration
    template <typename UserConfig, typename UserKeysToIgnore = void>
    class Table : public fhicl::ConfigurationTable {

      template <typename T>
      struct FullConfig {
        fhicl::Atom<std::string>  module_type { fhicl::Name("module_type") };
        fhicl::Atom<bool> errorOnFailureToPut { fhicl::Name("errorOnFailureToPut"), true };
        fhicl::TableFragment<T> user;
      };

      using KeysToIgnore_t = std::conditional_t<std::is_void<UserKeysToIgnore>::value,
                                                detail::IgnoreModuleLabel,
                                                fhicl::KeysToIgnore<detail::IgnoreModuleLabel, UserKeysToIgnore>>;

      fhicl::Table<FullConfig<UserConfig>, KeysToIgnore_t> fullConfig_;

      cet::exempt_ptr<fhicl::detail::ParameterBase const> get_parameter_base() const override { return &fullConfig_; }

    public:

      explicit Table(fhicl::Name&& name) : fullConfig_{std::move(name)} {}
      Table(fhicl::ParameterSet const& pset) : fullConfig_{pset} {}

      auto const& operator()() const { return fullConfig_().user(); }
      auto const& get_PSet() const { return fullConfig_.get_PSet(); }

      void print_allowed_configuration(std::ostream& os, std::string const& prefix) const
      {
        fullConfig_.print_allowed_configuration(os, prefix);
      }

    };

  };

  template <typename PROD, BranchType B>
  ProductID
  ProducerBase::getProductID(ModuleDescription const& md,
                             std::string const& instanceName) const
  {
    auto const& pd = get_ProductDescription<PROD>(B,
                                                 md.moduleLabel(),
                                                 instanceName);
    return pd.productID();
  }

  template <typename T>
  inline std::ostream& operator<<(std::ostream& os, ProducerBase::Table<T> const& t)
  {
    std::ostringstream config;
    t.print_allowed_configuration(config, std::string(3,' '));
    return os << config.str();
  }

}  // art

#endif /* art_Framework_Core_ProducerBase_h */

// Local Variables:
// mode: c++
// End:
