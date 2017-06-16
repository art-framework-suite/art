#ifndef art_Framework_Principal_Run_h
#define art_Framework_Principal_Run_h

// ======================================================================
//
// Run - This is the primary interface for accessing per run EDProducts
//       and inserting new derived products.
//
// For its usage, see "art/Framework/Principal/DataViewImpl.h"
// ======================================================================

#include "art/Framework/Principal/DataViewImpl.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ProductTokens.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Utilities/TypeID.h"

#include <memory>
#include <utility>

class art::Run final : private art::DataViewImpl {
public:

  using Base = DataViewImpl;

  explicit Run(RunPrincipal const& rp,
               ModuleDescription const& md,
               RangeSet const& rsForPuttingProducts = RangeSet::invalid());

  // AUX functions.
  RunID const& id() const {return aux_.id();}
  RunNumber_t run() const {return aux_.run();}
  Timestamp const& beginTime() const {return aux_.beginTime();}
  Timestamp const& endTime() const {return aux_.endTime();}

  // Retrieve a product
  using Base::get;
  using Base::getByLabel;
  using Base::getMany;
  using Base::getManyByType;
  using Base::getPointerByLabel;
  using Base::getValidHandle;

  // Retrieve a view to a collection of products
  using Base::getView;

  // Put a new product
  template <typename PROD> art::ProductID put(std::unique_ptr<PROD>&&);
  template <typename PROD> art::ProductID put(std::unique_ptr<PROD>&&, FullToken<Level::Run>);
  template <typename PROD> art::ProductID put(std::unique_ptr<PROD>&&, FragmentToken<Level::Run>);
  template <typename PROD> art::ProductID put(std::unique_ptr<PROD>&&, RangedFragmentToken<Level::Run>);

  // Put a new product with an instance name
  template <typename PROD> art::ProductID put(std::unique_ptr<PROD>&&, std::string const& instanceName);
  template <typename PROD> art::ProductID put(std::unique_ptr<PROD>&&, std::string const& instanceName, FullToken<Level::Run>);
  template <typename PROD> art::ProductID put(std::unique_ptr<PROD>&&, std::string const& instanceName, FragmentToken<Level::Run>);
  template <typename PROD> art::ProductID put(std::unique_ptr<PROD>&&, std::string const& instanceName, RangedFragmentToken<Level::Run>);

  // Expert-level
  using Base::removeCachedProduct;
  using Base::processHistory;

  // Return true if this Run has been subjected to a process with the
  // given processName, and false otherwise.  If true is returned,
  // then ps is filled with the ParameterSets (possibly more than one)
  // used to configure the identified process(es). Equivalent
  // ParameterSets are compressed out of the result.
  bool
  getProcessParameterSet(std::string const& processName,
                         std::vector<fhicl::ParameterSet>& ps) const;

private:
  // commit_() is called to complete the transaction represented by
  // this DataViewImpl. The friendships required are gross, but any
  // alternative is not great either.  Putting it into the
  // public interface is asking for trouble
  friend class InputSource;
  friend class DecrepitRelicInputSourceImplementation;
  friend class EDFilter;
  friend class EDProducer;

  void commit_(RunPrincipal&);

  ///Put a new product with a 'product instance name' and a 'range set'
  template <typename PROD>
  art::ProductID
  put_(std::unique_ptr<PROD> && product,
       std::string const& productInstanceName,
       RangeSet const& rs);

  RunAuxiliary const& aux_;
  RangeSet productRangeSet_;
};

//================================================================
// Implementation

//----------------------------------------------------------------
// putting with no specified instance name

template <typename PROD>
art::ProductID
art::Run::put(std::unique_ptr<PROD>&& product)
{
  return put<PROD>(std::move(product), std::string{});
}

template <typename PROD>
art::ProductID
art::Run::put(std::unique_ptr<PROD>&& product,
              FullToken<Level::Run> const token)
{
  return put<PROD>(std::move(product), std::string{}, token);
}

template <typename PROD>
art::ProductID
art::Run::put(std::unique_ptr<PROD>&& product,
              FragmentToken<Level::Run> const token)
{
  return put<PROD>(std::move(product), std::string{}, token);
}

template <typename PROD>
art::ProductID
art::Run::put(std::unique_ptr<PROD>&& product,
              RangedFragmentToken<Level::Run> token)
{
  return put<PROD>(std::move(product), std::string{}, std::move(token));
}

//----------------------------------------------------------------
// putting with specified instance name

template <typename PROD>
art::ProductID
art::Run::put(std::unique_ptr<PROD>&& product,
              std::string const& productInstanceName)
{
  productRangeSet_.collapse();
  return put_<PROD>(std::move(product), productInstanceName, productRangeSet_);
}

template <typename PROD>
art::ProductID
art::Run::put(std::unique_ptr<PROD>&& product,
              std::string const& productInstanceName,
              FullToken<Level::Run>)
{
  return put_<PROD>(std::move(product), productInstanceName, RangeSet::forRun(id()));
}

template <typename PROD>
art::ProductID
art::Run::put(std::unique_ptr<PROD>&& product,
              std::string const& productInstanceName,
              FragmentToken<Level::Run>)
{
  static_assert(detail::CanBeAggregated<PROD>::value,
                "\n\n"
                "art error: A Run product put with the token 'RunFragment'\n"
                "           must be able to be aggregated. Please add the appropriate\n"
                "              void aggregate(T const&)\n"
                "           function to your class, or contact artists@fnal.gov.\n");

  if (productRangeSet_.collapse().is_full_run()) {
    throw Exception{errors::ProductPutFailure, "Run::put"}
      << "\nCannot put a product corresponding to a full Run using\n"
      << "art::runFragment().  This can happen if you attempted to\n"
      << "put a product at beginRun using art::runFragment().\n"
      << "Please use either:\n"
      << "   art::fullRun(), or\n"
      << "   art::runFragment(art::RangeSet const&)\n"
      << "or contact artists@fnal.gov for assistance.\n";
  }
  return put_<PROD>(std::move(product), productInstanceName, productRangeSet_);
}

template <typename PROD>
art::ProductID
art::Run::put(std::unique_ptr<PROD>&& product,
              std::string const& productInstanceName,
              RangedFragmentToken<Level::Run> token)
{
  static_assert(detail::CanBeAggregated<PROD>::value,
                "\n\n"
                "art error: A Run product put with the token 'RunFragment'\n"
                "           must be able to be aggregated. Please add the appropriate\n"
                "              void aggregate(T const&)\n"
                "           function to your class, or contact artists@fnal.gov.\n");
  if (token.rs.collapse().is_full_run()) {
    throw Exception{errors::ProductPutFailure, "Run::put"}
      << "\nCannot put a product corresponding to a full Run using\n"
      << "art::runFragment(art::RangeSet&).  Please use:\n"
      << "   art::fullRun()\n"
      << "or contact artists@fnal.gov for assistance.\n";
  }
  return put_<PROD>(std::move(product), productInstanceName, token.rs);
}


template <typename PROD>
art::ProductID
art::Run::put_(std::unique_ptr<PROD>&& product,
               std::string const& productInstanceName,
               RangeSet const& rs)
{
  TypeID const tid{typeid(PROD)};
  if (product.get() == nullptr) {
    throw Exception{errors::NullPointerError, "Run::put"}
      << "\nA null unique_ptr was passed to 'put'.\n"
      << "The pointer is of type " << tid << ".\n"
      << "The specified productInstanceName was '"
      << productInstanceName << "'.\n";
  }

  if (!rs.is_valid()) {
    throw Exception{errors::ProductPutFailure, "Run::put"}
      << "\nCannot put a product with an invalid RangeSet.\n"
      << "Please contact artists@fnal.gov.\n";
  }

  auto const& bd = getBranchDescription(tid, productInstanceName);
  auto wp = std::make_unique<Wrapper<PROD>>(std::move(product));

  auto result = putProducts().emplace(TypeLabel{tid, productInstanceName},
                                      PMValue{std::move(wp), bd, rs});
  if (!result.second) {
    throw Exception{errors::ProductPutFailure, "Run::put"}
      << "\nAttempt to put multiple products with the\n"
      << "following description onto the Run.\n"
      << "Products must be unique per Run.\n"
      << "=================================\n"
      << bd
      << "=================================\n";
  }

  return bd.productID();
}

#endif /* art_Framework_Principal_Run_h */

// Local Variables:
// mode: c++
// End:
