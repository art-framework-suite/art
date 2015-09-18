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
#include "art/Utilities/TypeID.h"
#include "cpp0x/memory"
#include "cpp0x/utility"

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

#ifndef __GCCXML__
  ///Put a new product.
  template <typename PROD>
  void
  put(std::unique_ptr<PROD> && product) {put<PROD>(std::move(product), std::string());}

  ///Put a new product with a 'product instance name'
  template <typename PROD>
  void
  put(std::unique_ptr<PROD> && product, std::string const& productInstanceName);
#endif

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

#ifndef __GCCXML__
template <typename PROD>
void
art::Results::put(std::unique_ptr<PROD> && product, std::string const& productInstanceName) {
  if (product.get() == 0) {                // null pointer is illegal
    TypeID typeID(typeid(PROD));
    throw art::Exception(art::errors::NullPointerError)
      << "Results::put: A null unique_ptr was passed to 'put'.\n"
      << "The pointer is of type " << typeID << ".\n"
      << "The specified productInstanceName was '" << productInstanceName << "'.\n";
  }

  BranchDescription const& desc =
    getBranchDescription(TypeID(*product), productInstanceName);

  Wrapper<PROD> *wp(new Wrapper<PROD>(std::move(product)));

  putProducts().push_back(std::make_pair(wp, &desc));
}

#endif

#endif /* art_Framework_Principal_Results_h */

// Local Variables:
// mode: c++
// End:
