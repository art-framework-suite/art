#ifndef art_Framework_Principal_Results_h
#define art_Framework_Principal_Results_h

// ======================================================================
//
// Results: This is the primary interface for accessing results-level
// EDProducts and inserting new results-level EDProducts.
//
// For its usage, see "art/Framework/Principal/DataViewImpl.h"
//
// ======================================================================

#include "art/Framework/Principal/DataViewImpl.h"
#include "art/Framework/Principal/ProductInfo.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Common/traits.h"
#include "canvas/Utilities/TypeID.h"
#include <memory>
#include <utility>

namespace art {
  class Consumer;
  class Results;
}

class art::Results final : private art::DataViewImpl {
public:
  explicit Results(Principal const& p,
                   ModuleDescription const& md,
                   cet::exempt_ptr<Consumer> consumer);

  using Base = DataViewImpl;

  // Retrieve a product
  using Base::get;
  using Base::getByLabel;
  using Base::getMany;
  using Base::getManyByType;
  using Base::getPointerByLabel;
  using Base::getValidHandle;

  // Retrieve a view to a collection of products
  using Base::getView;

  // Put a new product.
  template <typename PROD>
  art::ProductID put(std::unique_ptr<PROD>&& product);

  // Put a new product with a 'product instance name'
  template <typename PROD>
  art::ProductID put(std::unique_ptr<PROD>&& product,
                     std::string const& productInstanceName);

  // Expert-level
  using Base::processHistory;
  using Base::removeCachedProduct;

  EDProductGetter const* productGetter(ProductID const pid) const;

  // In principle, the principal (heh, heh) need not be a function
  // argument since this class already keeps an internal reference to
  // it.  However, since the 'commit' function is public, requiring
  // the principal as an argument prevents a commit from being called
  // inappropriately.
  void commit(ResultsPrincipal&);

  template <typename T>
  using HandleT = Handle<T>;

private:
  Principal const& principal_;
};

template <typename PROD>
art::ProductID
art::Results::put(std::unique_ptr<PROD>&& product)
{
  return put<PROD>(std::move(product), {});
}

template <typename PROD>
art::ProductID
art::Results::put(std::unique_ptr<PROD>&& product,
                  std::string const& productInstanceName)
{
  TypeID const tid{typeid(PROD)};
  if (!product) { // Null pointer is illegal.
    throw art::Exception(art::errors::NullPointerError)
      << "Results::put: A null unique_ptr was passed to 'put'.\n"
      << "The pointer is of type " << tid << ".\n"
      << "The specified productInstanceName was '" << productInstanceName
      << "'.\n";
  }

  auto const& pd = getProductDescription(tid, productInstanceName);
  auto wp = std::make_unique<Wrapper<PROD>>(std::move(product));

  auto result = putProducts().emplace(
    TypeLabel{
      tid, productInstanceName, SupportsView<PROD>::value, false /*not used*/},
    PMValue{std::move(wp), pd, RangeSet::invalid()});
  if (!result.second) {
    throw art::Exception(art::errors::ProductPutFailure)
      << "Results::put: Attempt to put multiple products with the\n"
      << "              following description onto the Results.\n"
      << "              Products must be unique per Results.\n"
      << "=================================\n"
      << pd << "=================================\n";
  }

  return pd.productID();
}

#endif /* art_Framework_Principal_Results_h */

// Local Variables:
// mode: c++
// End:
