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
#include <tuple>
#include <vector>

namespace CLHEP {

  class HepRandomEngine;

} // namespace CLHEP

namespace fhicl {

  class ParameterSet;

} // namespace fhicl

namespace art {

  // class ProductInfo {
  //
  // public: // TYPES
  //
  //  enum class ConsumableType {
  //      Product // 0
  //    , ViewElement // 1
  //    , Many // 2
  //  };
  //
  // public: // MEMBER FUNCTIONS -- Special Member Functions
  //
  //  explicit
  //  ProductInfo(ConsumableType const, TypeID const&);
  //
  //  explicit
  //  ProductInfo(ConsumableType const, TypeID const&, std::string const& label,
  //  std::string const& instance,
  //              std::string const& process);
  //
  // public: // MEMBER DATA -- FIXME: Are these supposed to be public?
  //
  //  ConsumableType
  //  consumableType_;
  //
  //  TypeID
  //  typeID_;
  //
  //  std::string
  //  label_{};
  //
  //  std::string
  //  instance_{};
  //
  //  std::string
  //  process_{};
  //
  //};
  //
  // bool
  // operator<(ProductInfo const& a, ProductInfo const& b);

  // using ConsumableProductVectorPerBranch = std::vector<ProductInfo>;
  // using ConsumableProductSetPerBranch = std::set<ProductInfo>;
  // using ConsumableProducts = std::array<ConsumableProductVectorPerBranch,
  // NumBranchTypes>;  using ConsumableProductSets =
  // std::array<ConsumableProductSetPerBranch, NumBranchTypes>;

  class ModuleBase {

    // Allow the WorkerT<T> ctor to call setModuleDescription().
    // template <typename T> friend class WorkerT;

  public: // MEMBER FUNCTIONS -- Special Member Functions
    virtual ~ModuleBase();

    ModuleBase();

  public: // MEMBER FUNCTIONS -- API for the user
    ModuleDescription const& moduleDescription() const;

    int streamIndex() const;

    ModuleThreadingType moduleThreadingType() const;

    void setModuleDescription(ModuleDescription const&);

    void setStreamIndex(int streamIndex);

    hep::concurrency::SerialTaskQueueChain* serialTaskQueueChain() const;

  public: // MEMBER FUNCTIONS -- API for one modules
    void uses(std::string const& resourceName);

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
    // bool
    // getRequireConsumes() const;

    // void
    // setRequireConsumes(bool);

    std::array<std::vector<ProductInfo>, NumBranchTypes> const& getConsumables()
      const;

    void sortConsumables();

    // std::array<std::set<ProductInfo>, NumBranchTypes>&
    // getMissingConsumes();

    // This is used by doBeginJob().
    // void
    // prepareForJob(fhicl::ParameterSet const&);

    // This is used by get*() in DataViewImpl.
    // void
    // validateConsumedProduct(BranchType const, ProductInfo::ConsumableType,
    // std::type_info const&, std::string const& label,
    //                        std::string const& instance, std::string const&
    //                        process);

    //// This is used by doEndJob().
    // void
    // showMissingConsumes() const;

  protected: // MEMBER DATA -- For derived classes.
    ModuleDescription md_{};

    int streamIndex_{};

    ModuleThreadingType moduleThreadingType_{};

    std::set<std::string> resourceNames_{};

    std::unique_ptr<hep::concurrency::SerialTaskQueueChain> chain_{};

    // bool
    // requireConsumes_{false};

    std::array<std::vector<ProductInfo>, NumBranchTypes> consumables_{};

    // std::array<std::set<ProductInfo>, NumBranchTypes>
    // missingConsumes_{};
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

  //// FIXME: This is inline so that DataViewImpl which is in
  /// libart_Framework_Principal.so / FIXME: is not link-time dependent on
  /// libart_Framework_Core.so which contains us.
  // inline
  // void
  // ModuleBase::
  // validateConsumedProduct(BranchType const bt, ProductInfo::ConsumableType
  // ct, std::type_info const& ti, std::string const& label,
  //                        std::string const& instance, std::string const&
  //                        process)
  //{
  //  ProductInfo const pi{ct, TypeID(ti), label, instance, process};
  //  if (binary_search(consumables_[bt].cbegin(), consumables_[bt].cend(), pi))
  //  {
  //    // Found it, everything is ok.
  //    return;
  //  }
  //  if (requireConsumes_) {
  //    throw Exception(errors::ProductRegistrationFailure, "Consumer: an error
  //    occurred during validation of a retrieved product\n\n")
  //        << "The following consumes (or mayConsume) statement is missing
  //        from\n"
  //        << module_context(md_)
  //        << ":\n\n"
  //        << "  "
  //        << assemble_consumes_statement(bt, pi)
  //        << "\n\n";
  //  }
  //  missingConsumes_[bt].insert(pi);
  //}

} // namespace art

  // Local Variables:
  // mode: c++
  // End:

#endif /* art_Framework_Core_ModuleBase_h */
