#ifndef art_Framework_Core_ProducerBase_h
#define art_Framework_Core_ProducerBase_h

//----------------------------------------------------------------------
// ProducerBase: The base class of all "modules" that will insert new
//               EDProducts into an Event.
//----------------------------------------------------------------------

#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Core/detail/IgnoreModuleLabel.h"
#include "art/Framework/Core/get_BranchDescription.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ConfigurationTable.h"
#include "art/Utilities/ProductTokens.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "fhiclcpp/types/Atom.h"
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

    bool modifiesEvent() const { return true; }

    template <typename PROD, BranchType B, typename TRANS>
    ProductID getProductID(TRANS const &translator,
                           ModuleDescription const &moduleDescription,
                           std::string const& instanceName) const;

    // Configuration
    template <typename UserConfig, typename UserKeysToIgnore = void>
    class Table : public ConfigurationTable {

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

      Table(fhicl::Name&& name) : fullConfig_{std::move(name)} {}

      Table(fhicl::ParameterSet const& pset) : fullConfig_{pset} {}

      auto const& operator()() const { return fullConfig_().user(); }
      auto const& get_PSet() const { return fullConfig_.get_PSet(); }

      void print_allowed_configuration(std::ostream& os, std::string const& prefix) const
      {
        fullConfig_.print_allowed_configuration(os, prefix);
      }

    };

  };

  template <typename PROD, BranchType B, typename TRANS>
  ProductID
  ProducerBase::getProductID(TRANS const &translator,
                             ModuleDescription const &md,
                             std::string const &instanceName) const {
    return
      translator.branchIDToProductID
      (get_BranchDescription<PROD>(B,
                                   md.moduleLabel(),
                                   instanceName).branchID());

  }

  template <typename T, typename U>
  inline decltype(auto) operator<<(T&& t, ProducerBase::Table<U> const& u)
  {
    std::ostringstream oss;
    u.print_allowed_configuration(oss, std::string(3,' '));
    return std::forward<T>(t) << oss.str();
  }

}  // art

#endif /* art_Framework_Core_ProducerBase_h */

// Local Variables:
// mode: c++
// End:
