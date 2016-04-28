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
#include "art/Utilities/ProductTokens.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Utilities/TypeID.h"

#include <memory>
#include <utility>

class art::SubRun : private art::DataViewImpl {
public:

  SubRun(SubRunPrincipal& srp, const ModuleDescription& md);
  ~SubRun() = default;

  using Base = DataViewImpl;

  // AUX functions.
  SubRunNumber_t subRun() const {return aux_.subRun();}
  RunNumber_t run() const {return aux_.run();}
  SubRunID id() const { return aux_.id(); }

  Timestamp const& beginTime() const {return aux_.beginTime();}
  Timestamp const& endTime() const {return aux_.endTime();}

  RangeSet fullSubRunRangeSet() const { return RangeSet::forSubRun(id()); }
  RangeSet const& seenRangeSet() const { return seenRanges_; }

  using Base::get;
  using Base::getByLabel;
  using Base::getMany;
  using Base::getManyByType;
  using Base::removeCachedProduct;
  using Base::me;
  using Base::processHistory;

  template <typename PROD>
  art::ValidHandle<PROD> getValidHandle(InputTag const& tag) const
  {
    art::Handle<PROD> h;
    getByLabel(tag, h);
    return art::ValidHandle<PROD>(&(*h), *h.provenance());
  }

  Run const& getRun() const;

  ///Put a new product.
  template <typename PROD>
  void
  put(std::unique_ptr<PROD> && product,
      detail::RangedFragmentToken<Level::SubRun> const& token)
  {
    put<PROD>(std::move(product), std::string(), token);
  }

  ///Put a new product.
  template <typename PROD>
  void
  put(std::unique_ptr<PROD> && product,
      std::string const& productInstanceName,
      detail::RangedFragmentToken<Level::SubRun> const& token)
  {
    static_assert(detail::CanBeAggregated<PROD>::value,
                  "\n\n"
                  "art error: A SubRun product put with the token 'SubRunFragment'\n"
                  "           must be able to be aggregated. Please add the appropriate\n"
                  "              void aggregate(T const&)\n"
                  "           function to your class, or contact artists@fnal.gov.\n");
    put_<PROD>(std::move(product), productInstanceName, token.rs);
  }


  template <typename PROD>
  void
  put(std::unique_ptr<PROD> && product, detail::FullToken<Level::SubRun> const token)
  {
    put<PROD>(std::move(product), std::string(), token);
  }

  template <typename PROD>
  void
  put(std::unique_ptr<PROD> && product, detail::FragmentToken<Level::SubRun> const token)
  {
    put<PROD>(std::move(product), std::string(), token);
  }

  template <typename PROD>
  void
  put(std::unique_ptr<PROD> && product,
      std::string const& productInstanceName,
      detail::FullToken<Level::SubRun>)
  {
    put_<PROD>(std::move(product), productInstanceName, fullSubRunRangeSet());
  }

  template <typename PROD>
  void
  put(std::unique_ptr<PROD> && product,
      std::string const& productInstanceName,
      detail::FragmentToken<Level::SubRun>)
  {
    static_assert(detail::CanBeAggregated<PROD>::value,
                  "\n\n"
                  "art error: A SubRun product put with the token 'SubRunFragment'\n"
                  "           must be able to be aggregated. Please add the appropriate\n"
                  "              void aggregate(T const&)\n"
                  "           function to your class, or contact artists@fnal.gov.\n");
    put_<PROD>(std::move(product), productInstanceName, seenRangeSet());
  }

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

  ///Put a new product with a 'product instance name' and a 'range set'
  template <typename PROD>
  void
  put_(std::unique_ptr<PROD> && product,
       std::string const& productInstanceName,
       RangeSet const& rs);

  SubRunAuxiliary const& aux_;
  std::shared_ptr<Run const> const run_;
  RangeSet seenRanges_;
};

template <typename PROD>
void
art::SubRun::put_(std::unique_ptr<PROD> && product,
                  std::string const& productInstanceName,
                  RangeSet const& rs)
{
  if (product.get() == nullptr) {
    throw art::Exception{art::errors::NullPointerError}
      << "SubRun::put: A null unique_ptr was passed to 'put'.\n"
      << "The pointer is of type " << TypeID{typeid(PROD)} << ".\n"
      << "The specified productInstanceName was '" << productInstanceName << "'.\n";
  }

  auto const& bd = getBranchDescription(TypeID(*product), productInstanceName);
  auto        wp = std::make_unique<Wrapper<PROD>>(std::move(product)/*,aux_.rangeSetID()*/);

  auto result = putProducts().emplace( bd.branchID(), PMValue{std::move(wp), bd, rs} );
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

#endif /* art_Framework_Principal_SubRun_h */

// Local Variables:
// mode: c++
// End:
