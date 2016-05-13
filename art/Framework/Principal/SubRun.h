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

  using Base = DataViewImpl;

  SubRun(SubRunPrincipal& srp,
         ModuleDescription const& md,
         RangeSet const& rsForPuttingProducts = RangeSet::invalid());

  SubRunNumber_t subRun() const {return aux_.subRun();}
  RunNumber_t run() const {return aux_.run();}
  SubRunID id() const { return aux_.id(); }

  Timestamp const& beginTime() const {return aux_.beginTime();}
  Timestamp const& endTime() const {return aux_.endTime();}

  using Base::get;
  using Base::getByLabel;
  using Base::getMany;
  using Base::getManyByType;
  using Base::removeCachedProduct;
  using Base::me;
  using Base::processHistory;

  template <typename PROD>
  art::ValidHandle<PROD>
  getValidHandle(InputTag const& tag) const;

  Run const& getRun() const;

  template <typename PROD> void put(std::unique_ptr<PROD>&&);
  template <typename PROD> void put(std::unique_ptr<PROD>&&, FullToken<Level::SubRun>);
  template <typename PROD> void put(std::unique_ptr<PROD>&&, FragmentToken<Level::SubRun>);
  template <typename PROD> void put(std::unique_ptr<PROD>&&, RangedFragmentToken<Level::SubRun>);

  template <typename PROD> void put(std::unique_ptr<PROD>&&, std::string const& instanceName);
  template <typename PROD> void put(std::unique_ptr<PROD>&&, std::string const& instanceName, FullToken<Level::SubRun>);
  template <typename PROD> void put(std::unique_ptr<PROD>&&, std::string const& instanceName, FragmentToken<Level::SubRun>);
  template <typename PROD> void put(std::unique_ptr<PROD>&&, std::string const& instanceName, RangedFragmentToken<Level::SubRun>);


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
  put_(std::unique_ptr<PROD>&& product,
       std::string const& productInstanceName,
       RangeSet const& rs);

  SubRunAuxiliary const& aux_;
  std::shared_ptr<Run const> const run_;
  RangeSet productRangeSet_;
};

//================================================================
// Implementation

template <typename PROD>
art::ValidHandle<PROD>
art::SubRun::getValidHandle(InputTag const& tag) const
{
  art::Handle<PROD> h;
  getByLabel(tag, h);
  return art::ValidHandle<PROD>(&(*h), *h.provenance());
}

//----------------------------------------------------------------
// putting with no specified instance name

template <typename PROD>
void
art::SubRun::put(std::unique_ptr<PROD>&& product)
{
  put<PROD>(std::move(product), std::string{});
}

template <typename PROD>
void
art::SubRun::put(std::unique_ptr<PROD>&& product,
                 FullToken<Level::SubRun> const token)
{
  put<PROD>(std::move(product), std::string{}, token);
}

template <typename PROD>
void
art::SubRun::put(std::unique_ptr<PROD>&& product,
                 FragmentToken<Level::SubRun> const token)
{
  put<PROD>(std::move(product), std::string{}, token);
}

template <typename PROD>
void
art::SubRun::put(std::unique_ptr<PROD>&& product,
                 RangedFragmentToken<Level::SubRun> token)
{
  put<PROD>(std::move(product), std::string{}, std::move(token));
}

//----------------------------------------------------------------
// putting with specified instance name

template <typename PROD>
void
art::SubRun::put(std::unique_ptr<PROD>&& product,
                 std::string const& productInstanceName)
{
  productRangeSet_.collapse();
  put_<PROD>(std::move(product), productInstanceName, productRangeSet_);
}

template <typename PROD>
void
art::SubRun::put(std::unique_ptr<PROD>&& product,
                 std::string const& productInstanceName,
                 FullToken<Level::SubRun>)
{
  put_<PROD>(std::move(product), productInstanceName, RangeSet::forSubRun(id()));
}

template <typename PROD>
void
art::SubRun::put(std::unique_ptr<PROD>&& product,
                 std::string const& productInstanceName,
                 FragmentToken<Level::SubRun>)
{
  static_assert(detail::CanBeAggregated<PROD>::value,
                "\n\n"
                "art error: A SubRun product put with the token 'SubRunFragment'\n"
                "           must be able to be aggregated. Please add the appropriate\n"
                "              void aggregate(T const&)\n"
                "           function to your class, or contact artists@fnal.gov.\n");

  if (productRangeSet_.collapse().is_full_subRun()) {
    throw art::Exception(art::errors::InsertFailure, "SubRun::put")
      << "\nCannot put a product corresponding to a full SubRun using\n"
      << "art::subRunFragment().  This can happen if you attempted to\n"
      << "put a product at beginSubRun using art::subRunFragment().\n"
      << "Please use either:\n"
      << "   art::fullSubRun(), or\n"
      << "   art::subRunFragment(art::RangeSet const&)\n"
      << "or contact artists@fnal.gov for assistance.\n";
  }
  put_<PROD>(std::move(product), productInstanceName, productRangeSet_);
}

template <typename PROD>
void
art::SubRun::put(std::unique_ptr<PROD>&& product,
                 std::string const& productInstanceName,
                 RangedFragmentToken<Level::SubRun> token)
{
  static_assert(detail::CanBeAggregated<PROD>::value,
                "\n\n"
                "art error: A SubRun product put with the token 'SubRunFragment'\n"
                "           must be able to be aggregated. Please add the appropriate\n"
                "              void aggregate(T const&)\n"
                "           function to your class, or contact artists@fnal.gov.\n");
  if (token.rs.collapse().is_full_subRun()) {
    throw Exception{errors::InsertFailure, "Run::put"}
      << "\nCannot put a product corresponding to a full SubRun using\n"
      << "art::subRunFragment(art::RangeSet&).  Please use:\n"
      << "   art::fullSubRun()\n"
      << "or contact artists@fnal.gov for assistance.\n";
  }
  put_<PROD>(std::move(product), productInstanceName, token.rs);
}


template <typename PROD>
void
art::SubRun::put_(std::unique_ptr<PROD>&& product,
                  std::string const& productInstanceName,
                  RangeSet const& rs)
{
  if (product.get() == nullptr) {
    throw art::Exception{art::errors::NullPointerError, "SubRun::put"}
      << "\nA null unique_ptr was passed to 'put'.\n"
      << "The pointer is of type " << TypeID{typeid(PROD)} << ".\n"
      << "The specified productInstanceName was '" << productInstanceName << "'.\n";
  }

  if (!rs.is_valid()) {
    throw art::Exception{art::errors::InsertFailure, "SubRun::put"}
      << "\nCannot put a product with an invalid RangeSet.\n"
      << "Please contact artists@fnal.gov.\n";
  }

  auto const& bd = getBranchDescription(TypeID(*product), productInstanceName);
  auto wp = std::make_unique<Wrapper<PROD>>(std::move(product));

  auto result = putProducts().emplace(bd.branchID(), PMValue{std::move(wp), bd, rs});
  if (!result.second) {
    throw art::Exception{art::errors::InsertFailure, "SubRun::put"}
      << "\nAttempt to put multiple products with the\n"
      << "following description onto the SubRun.\n"
      << "Products must be unique per SubRun.\n"
      << "=================================\n"
      << bd
      << "=================================\n";
  }
}

#endif /* art_Framework_Principal_SubRun_h */

// Local Variables:
// mode: c++
// End:
