#ifndef art_Framework_Core_ModuleBase_h
#define art_Framework_Core_ModuleBase_h
// vim: set sw=2 expandtab :

#include "art/Utilities/Globals.h" // for ProductInfo
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProductToken.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/exempt_ptr.h"

#include <array>
#include <string>
#include <vector>

namespace CLHEP {
  class HepRandomEngine;
}

namespace fhicl {
  class ParameterSet;
}

namespace art {
  class ModuleBase {
  public:
    virtual ~ModuleBase() noexcept;
    ModuleBase();

    ModuleDescription const& moduleDescription() const;
    void setModuleDescription(ModuleDescription const&);

    // RandomNumberGenerator interface
    CLHEP::HepRandomEngine& createEngine(long);
    CLHEP::HepRandomEngine& createEngine(
      long,
      std::string const& kind_of_engine_to_make);
    CLHEP::HepRandomEngine& createEngine(
      long,
      std::string const& kind_of_engine_to_make,
      std::string const& engine_label);
    long get_seed_value(fhicl::ParameterSet const&,
                        char const key[] = "seed",
                        long const implicit_seed = -1);

    // Consumes information
    template <typename T, BranchType = InEvent>
    ProductToken<T> consumes(InputTag const&);
    template <typename Element, BranchType = InEvent>
    ViewToken<Element> consumesView(InputTag const&);
    template <typename T, BranchType = InEvent>
    void consumesMany();

    template <typename T, BranchType = InEvent>
    ProductToken<T> mayConsume(InputTag const&);
    template <typename Element, BranchType = InEvent>
    ViewToken<Element> mayConsumeView(InputTag const&);
    template <typename T, BranchType = InEvent>
    void mayConsumeMany();

    std::array<std::vector<ProductInfo>, NumBranchTypes> const& getConsumables()
      const;
    void sortConsumables();

  private:
    ModuleDescription md_{};
    std::array<std::vector<ProductInfo>, NumBranchTypes> consumables_{};
  };

  template <typename T, BranchType BT>
  ProductToken<T>
  ModuleBase::consumes(InputTag const& tag)
  {
    consumables_[BT].emplace_back(ProductInfo::ConsumableType::Product,
                                  TypeID{typeid(T)},
                                  tag.label(),
                                  tag.instance(),
                                  tag.process());
    return ProductToken<T>{tag};
  }

  template <typename T, BranchType BT>
  ViewToken<T>
  ModuleBase::consumesView(InputTag const& tag)
  {
    consumables_[BT].emplace_back(ProductInfo::ConsumableType::ViewElement,
                                  TypeID{typeid(T)},
                                  tag.label(),
                                  tag.instance(),
                                  tag.process());
    return ViewToken<T>{tag};
  }

  template <typename T, BranchType BT>
  void
  ModuleBase::consumesMany()
  {
    consumables_[BT].emplace_back(ProductInfo::ConsumableType::Many,
                                  TypeID{typeid(T)});
  }

  template <typename T, BranchType BT>
  ProductToken<T>
  ModuleBase::mayConsume(InputTag const& tag)
  {
    consumables_[BT].emplace_back(ProductInfo::ConsumableType::Product,
                                  TypeID{typeid(T)},
                                  tag.label(),
                                  tag.instance(),
                                  tag.process());
    return ProductToken<T>{tag};
  }

  template <typename T, BranchType BT>
  ViewToken<T>
  ModuleBase::mayConsumeView(InputTag const& tag)
  {
    consumables_[BT].emplace_back(ProductInfo::ConsumableType::ViewElement,
                                  TypeID{typeid(T)},
                                  tag.label(),
                                  tag.instance(),
                                  tag.process());
    return ViewToken<T>{tag};
  }

  template <typename T, BranchType BT>
  void
  ModuleBase::mayConsumeMany()
  {
    consumables_[BT].emplace_back(ProductInfo::ConsumableType::Many,
                                  TypeID{typeid(T)});
  }
} // namespace art

  // Local Variables:
  // mode: c++
  // End:

#endif /* art_Framework_Core_ModuleBase_h */
