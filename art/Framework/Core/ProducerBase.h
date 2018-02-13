#ifndef art_Framework_Core_ProducerBase_h
#define art_Framework_Core_ProducerBase_h

//----------------------------------------------------------------------
// ProducerBase: The base class of all "modules" that will insert new
//               EDProducts into an Event.
//----------------------------------------------------------------------

#include "art/Framework/Core/ProducerTable.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Core/detail/ImplicitConfigs.h"
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
#include <sstream>
#include <string>

namespace art {

  class BranchDescription;
  class ModuleDescription;

  class ProducerBase : private ProductRegistryHelper {
  public:
    using ProductRegistryHelper::expectedProducts;
    using ProductRegistryHelper::produces;
    using ProductRegistryHelper::registerProducts;

    bool
    modifiesEvent() const
    {
      return true;
    }

    template <typename PROD, BranchType B>
    ProductID getProductID(ModuleDescription const& moduleDescription,
                           std::string const& instanceName) const;

    template <typename UserConfig, typename UserKeysToIgnore = void>
    using Table =
      ProducerTable<UserConfig, detail::ModuleConfig, UserKeysToIgnore>;
  };

  template <typename PROD, BranchType B>
  ProductID
  ProducerBase::getProductID(ModuleDescription const& md,
                             std::string const& instanceName) const
  {
    auto const& pd =
      get_ProductDescription<PROD>(B, md.moduleLabel(), instanceName);
    return pd.productID();
  }

  template <typename T>
  inline std::ostream&
  operator<<(std::ostream& os, ProducerBase::Table<T> const& t)
  {
    std::ostringstream config;
    t.print_allowed_configuration(config, std::string(3, ' '));
    return os << config.str();
  }

} // art

#endif /* art_Framework_Core_ProducerBase_h */

// Local Variables:
// mode: c++
// End:
