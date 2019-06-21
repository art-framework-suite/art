#ifndef art_Framework_Core_Modifier_h
#define art_Framework_Core_Modifier_h
// vim: set sw=2 expandtab :

//============================================================
// The base class of all modules that will insert products.
//============================================================

#include "art/Framework/Core/ModuleBase.h"
#include "art/Framework/Core/ProducerTable.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Core/detail/ImplicitConfigs.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ModuleType.h"
#include "art/Utilities/ProductSemantics.h"
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

  class Modifier : public ModuleBase, private ProductRegistryHelper {
  public:
    template <typename UserConfig, typename UserKeysToIgnore = void>
    using Table =
      ProducerTable<UserConfig, detail::ModuleConfig, UserKeysToIgnore>;

    ~Modifier() noexcept;
    Modifier();

    Modifier(Modifier const&) = delete;
    Modifier(Modifier&&) = delete;
    Modifier& operator=(Modifier const&) = delete;
    Modifier& operator=(Modifier&&) = delete;

    using ProductRegistryHelper::fillDescriptions;
    using ProductRegistryHelper::registerProducts;

  protected:
    using ProductRegistryHelper::expectedProducts;
    using ProductRegistryHelper::produces;
    using ProductRegistryHelper::producesCollector;
  };

} // namespace art

#endif /* art_Framework_Core_Modifier_h */

// Local Variables:
// mode: c++
// End:
