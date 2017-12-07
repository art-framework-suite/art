#ifndef art_Framework_Core_ModuleBase_h
#define art_Framework_Core_ModuleBase_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/ModuleType.h"
#include "art/Utilities/Globals.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProductToken.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/exempt_ptr.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <array>
#include <memory>
#include <set>
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

    // Allow the WorkerT<T> ctor to call setModuleDescription().
    // template <typename T> friend class WorkerT;

  public: // MEMBER FUNCTIONS -- Special Member Functions
    virtual ~ModuleBase() noexcept;

    ModuleBase();

  public: // MEMBER FUNCTIONS -- API for the user
    ModuleDescription const& moduleDescription() const;

    int streamIndex() const;

    ModuleThreadingType moduleThreadingType() const;

    void setModuleDescription(ModuleDescription const&);

    void setStreamIndex(int streamIndex);

    hep::concurrency::SerialTaskQueueChain* serialTaskQueueChain() const;

  public: // MEMBER FUNCTIONS -- API for one modules

    template <typename... T>
    void serialize(T const&...);

    // FIXME: need "async<Level>()" function for opting in to
    // concurrent event processing for a given module.

  public: // MEMBER FUNCTIONS -- API for access to RandomNumberGenerator
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

  public: // MEMBER FUNCTIONS -- API for declaring consumes information
    template <typename T, BranchType = InEvent>
    ProductToken<T> consumes(InputTag const&);

    template <typename Element, BranchType = InEvent>
    ViewToken<Element> consumesView(InputTag const&);

    template <typename T, BranchType = InEvent>
    void consumesMany();

  public: // MEMBER FUNCTIONS -- API for declaring may consumes information
    template <typename T, BranchType = InEvent>
    ProductToken<T> mayConsume(InputTag const&);

    template <typename Element, BranchType = InEvent>
    ViewToken<Element> mayConsumeView(InputTag const&);

    template <typename T, BranchType = InEvent>
    void mayConsumeMany();

  public: // MEMBER FUNCTIONS -- API for using collected consumes information
    std::array<std::vector<ProductInfo>, NumBranchTypes> const& getConsumables()
      const;

    void sortConsumables();

  protected: // MEMBER DATA -- For derived classes.

    ModuleDescription md_{};
    int streamIndex_{};
    ModuleThreadingType moduleThreadingType_{};
    std::set<std::string> resourceNames_{};
    std::unique_ptr<hep::concurrency::SerialTaskQueueChain> chain_{};
    std::array<std::vector<ProductInfo>, NumBranchTypes> consumables_{};

  private:

    void serialize_for_resource();
    void serialize_for_resource(std::string const&);

    template <typename H, typename... T>
    std::enable_if_t<std::is_same<std::string,H>::value>
    serialize_for_resource(H const& head, T const&... tail)
    {
      serialize_for_resource(head);
      serialize_for_resource(tail...);
    }

  };

  template <typename... T>
  void
  ModuleBase::serialize(T const&... resources)
  {
    serialize_for_resource(resources...);
  }

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
