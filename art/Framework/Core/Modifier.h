#ifndef art_Framework_Core_Modifier_h
#define art_Framework_Core_Modifier_h
// vim: set sw=2 expandtab :

//============================================================
// The base class of all modules that will insert products.
//============================================================

#include "art/Framework/Core/ModuleBase.h"
#include "art/Framework/Core/ModuleType.h"
#include "art/Framework/Core/ProducerTable.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Core/detail/ImplicitConfigs.h"
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

  class Modifier : public ModuleBase, private ProductRegistryHelper {
  public: // CONFIGURATION
    template <typename UserConfig, typename UserKeysToIgnore = void>
    using Table =
      ProducerTable<UserConfig, detail::ModuleConfig, UserKeysToIgnore>;

  public: // MEMBER FUNCTIONS -- Special Member Functions
    virtual ~Modifier() noexcept;

    Modifier();

    Modifier(Modifier const&) = delete;
    Modifier(Modifier&&) = delete;
    Modifier& operator=(Modifier const&) = delete;
    Modifier& operator=(Modifier&&) = delete;

  public: // MEMBER FUNCTIONS -- Product Registry Helper API
    using ProductRegistryHelper::expectedProducts;
    using ProductRegistryHelper::fillDescriptions;
    using ProductRegistryHelper::produces;
    using ProductRegistryHelper::registerProducts;
  };

} // namespace art

#endif /* art_Framework_Core_Modifier_h */

// Local Variables:
// mode: c++
// End:
