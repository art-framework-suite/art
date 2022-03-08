#ifndef art_Framework_Principal_ProductInserter_h
#define art_Framework_Principal_ProductInserter_h
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/detail/type_label_for.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/fwd.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Common/traits.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Persistency/Provenance/fwd.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib_except/exception.h"

#include <cassert>
#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace art {

  class ProductInserter {
  public:
    ~ProductInserter();
    explicit ProductInserter(BranchType bt,
                             Principal& p,
                             ModuleContext const& mc);

    ProductInserter(ProductInserter const&) = delete;
    ProductInserter& operator=(ProductInserter const&) = delete;
    ProductInserter(ProductInserter&&) = default;
    ProductInserter& operator=(ProductInserter&&) = default;

    // Product insertion - all processing levels
    template <typename PROD>
    PutHandle<PROD> put(std::unique_ptr<PROD>&& edp,
                        std::string const& instance = {});

    // Product insertion -- run/subrun
    template <typename PROD>
    PutHandle<PROD> put(std::unique_ptr<PROD>&& edp,
                        std::string const& instance,
                        RangeSet const& rs);

    void commitProducts();
    void commitProducts(
      bool checkProducts,
      std::map<TypeLabel, BranchDescription> const* expectedProducts,
      std::vector<ProductID> retrievedPIDs);

  private:
    struct PMValue {
      std::unique_ptr<EDProduct> product;
      BranchDescription const& description;
      RangeSet rangeSet;
    };

    BranchDescription const& getProductDescription_(
      TypeID const& type,
      std::string const& instance,
      bool const alwaysEnableLookupOfProducedProducts = false) const;

    EDProductGetter const* productGetter_(ProductID id) const;
    Provenance provenance_(ProductID id) const;

    // Protects use of retrievedProducts_ and putProducts_.
    mutable std::unique_ptr<std::recursive_mutex> mutex_{
      std::make_unique<std::recursive_mutex>()};

    // Is this an Event, a Run, a SubRun, or a Results.
    BranchType branchType_;

    // The principal we are operating on.
    Principal* principal_;

    // The module we were created for.
    ModuleDescription const* md_;

    // The set of products which have been put by the user.
    std::map<TypeLabel, PMValue> putProducts_{};
  };

  // =======================================================================
  // Product-insertion implementation
  template <typename PROD>
  PutHandle<PROD>
  ProductInserter::put(std::unique_ptr<PROD>&& edp, std::string const& instance)
  {
    return put(move(edp), instance, RangeSet::invalid());
  }

  template <typename PROD>
  PutHandle<PROD>
  ProductInserter::put(std::unique_ptr<PROD>&& edp,
                       std::string const& instance,
                       RangeSet const& rs)
  {
    TypeID const tid{typeid(PROD)};
    if (!edp) {
      throw Exception(errors::NullPointerError)
        << "A null unique_ptr was passed to 'put'.\n"
        << "The pointer is of type " << tid << ".\n"
        << "The specified productInstanceName was '" << instance << "'.\n";
    }

    std::lock_guard lock{*mutex_};
    auto const& bd = getProductDescription_(tid, instance, true);
    assert(bd.productID() != ProductID::invalid());
    auto const typeLabel =
      detail::type_label_for(tid, instance, SupportsView<PROD>::value, *md_);
    auto wp = std::make_unique<Wrapper<PROD>>(move(edp));

    // Mind the product ownership!  The wrapper is the final resting
    // place of the product before it is taken out of memory.
    cet::exempt_ptr<PROD const> product{wp->product()};
    auto result =
      putProducts_.try_emplace(typeLabel, PMValue{move(wp), bd, rs}).second;
    if (!result) {
      constexpr cet::HorizontalRule rule{30};
      throw Exception(errors::ProductPutFailure)
        << "Attempt to put multiple products with the following descriptions.\n"
        << "Each product must be unique.\n"
        << rule('=') << '\n'
        << bd << rule('=') << '\n';
    }
    return PutHandle{
      product.get(), productGetter_(bd.productID()), bd.productID()};
  }

} // namespace art

#endif /* art_Framework_Principal_ProductInserter_h */

// Local Variables:
// mode: c++
// End:
