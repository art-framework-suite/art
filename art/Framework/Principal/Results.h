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
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Utilities/TypeID.h"
#include <memory>
#include <utility>

namespace art {
  class Results;
}

class art::Results : private art::DataViewImpl {
public:
  Results(ResultsPrincipal& srp, const ModuleDescription& md);
  ~Results() {}

  typedef DataViewImpl Base;

  using Base::get;
  using Base::getByLabel;
  using Base::getMany;
  using Base::getManyByType;
  using Base::removeCachedProduct;
  using Base::me;
  using Base::processHistory;

  ///Put a new product.
  template <typename PROD>
  void
  put(std::unique_ptr<PROD> && product) {put<PROD>(std::move(product), std::string());}

  ///Put a new product with a 'product instance name'
  template <typename PROD>
  void
  put(std::unique_ptr<PROD> && product, std::string const& productInstanceName);

private:

  // commit_() is called to complete the transaction represented by
  // this DataViewImpl. The friendships required are gross, but any
  // alternative is not great either.  Putting it into the
  // public interface is asking for trouble
  friend class InputSource;
  friend class DecrepitRelicInputSourceImplementation;
  friend class ResultsProducer;

  void commit_();
};

template <typename PROD>
void
art::Results::put(std::unique_ptr<PROD> && product, std::string const& productInstanceName) {

  if (!product) { // Null pointer is illegal.
    throw art::Exception(art::errors::NullPointerError)
      << "Results::put: A null unique_ptr was passed to 'put'.\n"
      << "The pointer is of type " << TypeID{typeid(PROD)} << ".\n"
      << "The specified productInstanceName was '" << productInstanceName << "'.\n";
  }

  auto const & bd = getBranchDescription(TypeID(*product), productInstanceName);
  auto  wp = std::make_unique<Wrapper<PROD> >(std::move(product));

  auto result = putProducts().emplace(bd.branchID(), PMValue { std::move(wp), bd, RangeSet::invalid() });
  if (!result.second) {
    throw art::Exception(art::errors::InsertFailure)
      << "Results::put: Attempt to put multiple products with the\n"
      << "              following description onto the Results.\n"
      << "              Products must be unique per Results.\n"
      << "=================================\n"
      << bd
      << "=================================\n";
  }
}

#endif /* art_Framework_Principal_Results_h */

// Local Variables:
// mode: c++
// End:
