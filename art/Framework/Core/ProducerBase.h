#ifndef art_Framework_Core_ProducerBase_h
#define art_Framework_Core_ProducerBase_h

//----------------------------------------------------------------------
// ProducerBase: The base class of all "modules" that will insert new
//               EDProducts into an Event.
//----------------------------------------------------------------------

#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Core/get_BranchDescription.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ProductTokens.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "fhiclcpp/types/Atom.h"
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
    template <typename T>
    struct FullConfig {
      fhicl::Atom<std::string>  module_type { fhicl::Name("module_type") };
      fhicl::Atom<bool> errorOnFailureToPut { fhicl::Name("errorOnFailureToPut"), true };
      fhicl::TableFragment<T> user;
    };

    template < typename UserConfig >
    class Table {
    public:

      Table() = default;

      Table(fhicl::ParameterSet const& pset) : Table()
      {
        std::set<std::string> const keys_to_ignore = { "module_label" };
        fullConfig_.validate_ParameterSet( pset, keys_to_ignore );
      }

      auto const& operator()() const { return fullConfig_().user(); }

      fhicl::ParameterSet const & get_PSet() const { return fullConfig_.get_PSet(); }

      void print_allowed_configuration (std::ostream& os, std::string const& prefix) const
      {
        fullConfig_.print_allowed_configuration(os, prefix);
      }

    private:
      fhicl::Table<FullConfig<UserConfig>> fullConfig_ { fhicl::Name("<module_label>") };
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
