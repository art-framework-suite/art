#ifndef art_Framework_Principal_SubRun_h
#define art_Framework_Principal_SubRun_h

// ======================================================================
//
// SubRun: This is the primary interface for accessing per subRun
// EDProducts and inserting new derived per subRun EDProducts.
//
// For its usage, see "art/Framework/Principal/DataViewImpl.h"
//
// ======================================================================

#include "art/Framework/Principal/DataViewImpl.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Utilities/TypeID.h"

#include <memory>
#include <utility>

class art::SubRun : private art::DataViewImpl {
public:
  SubRun(SubRunPrincipal& srp, const ModuleDescription& md);
  ~SubRun() {}

  typedef DataViewImpl Base;
  // AUX functions.
  SubRunNumber_t subRun() const {return aux_.subRun();}

  RunNumber_t run() const {return aux_.run();}

  SubRunID id() const {
    return aux_.id();
  }

  Timestamp const& beginTime() const {return aux_.beginTime();}
  Timestamp const& endTime() const {return aux_.endTime();}

  using Base::get;
  using Base::getByLabel;
  using Base::getMany;
  using Base::getManyByType;
  using Base::removeCachedProduct;
  using Base::me;
  using Base::processHistory;

  Run const&
  getRun() const;

#ifndef __GCCXML__
  ///Put a new product.
  template <typename PROD>
  void
  put(std::unique_ptr<PROD> && product) {put<PROD>(std::move(product), std::string());}

  ///Put a new product with a 'product instance name'
  template <typename PROD>
  void
  put(std::unique_ptr<PROD> && product, std::string const& productInstanceName);
#endif /* __GCCXML__ */

private:
  SubRunPrincipal const&
  subRunPrincipal() const;

  SubRunPrincipal &
  subRunPrincipal();

  // commit_() is called to complete the transaction represented by
  // this DataViewImpl. The friendships required are gross, but any
  // alternative is not great either.  Putting it into the
  // public interface is asking for trouble
  friend class InputSource;
  friend class DecrepitRelicInputSourceImplementation;
  friend class EDFilter;
  friend class EDProducer;

  void commit_();

  SubRunAuxiliary const& aux_;
  std::shared_ptr<Run const> const run_;
};

#ifndef __GCCXML__
template <typename PROD>
void
art::SubRun::put(std::unique_ptr<PROD> && product,
                 std::string const& productInstanceName)
{
  if (product.get() == nullptr) {
    throw art::Exception(art::errors::NullPointerError)
      << "SubRun::put: A null unique_ptr was passed to 'put'.\n"
      << "The pointer is of type " << TypeID(typeid(PROD)) << ".\n"
      << "The specified productInstanceName was '" << productInstanceName << "'.\n";
  }

  auto const& bd = getBranchDescription(TypeID(*product), productInstanceName);
  auto        wp = std::make_unique<Wrapper<PROD>>(std::move(product));

  auto result = putProducts().emplace( bd.branchID(), PMValue{std::move(wp), bd} );
  if ( !result.second ) {
    throw art::Exception(art::errors::InsertFailure)
      << "SubRun::put: Attempt to put multiple products with the\n"
      << "             following description onto the SubRun.\n"
      << "             Products must be unique per SubRun.\n"
      << "=================================\n"
      << bd
      << "=================================\n";
  }
}

#endif /* __GCCXML__ */

#endif /* art_Framework_Principal_SubRun_h */

// Local Variables:
// mode: c++
// End:
