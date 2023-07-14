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

namespace art {

  class Modifier : public ModuleBase, private ProductRegistryHelper {
  public:
    template <typename UserConfig, typename... UserKeysToIgnore>
    using Table =
      ProducerTable<UserConfig, detail::ModuleConfig, UserKeysToIgnore...>;

    ~Modifier() noexcept;
    Modifier();

    Modifier(Modifier const&) = delete;
    Modifier(Modifier&&) = delete;
    Modifier& operator=(Modifier const&) = delete;
    Modifier& operator=(Modifier&&) = delete;

    void fillProductDescriptions();
    void registerProducts(ProductDescriptions& productsToRegister);

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
