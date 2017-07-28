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
#include "art/Framework/Principal/ProductInfo.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ProductSemantics.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Utilities/TypeID.h"

#include <memory>
#include <utility>

namespace art {
  class Consumer;
}

class art::SubRun final : private art::DataViewImpl {
public:

  using Base = DataViewImpl;

  SubRun(SubRunPrincipal const& srp,
         ModuleDescription const& md,
         cet::exempt_ptr<Consumer> consumer,
         RangeSet const& rsForPuttingProducts = RangeSet::invalid());

  SubRunNumber_t subRun() const {return aux_.subRun();}
  RunNumber_t run() const {return aux_.run();}
  SubRunID id() const { return aux_.id(); }

  Timestamp const& beginTime() const {return aux_.beginTime();}
  Timestamp const& endTime() const {return aux_.endTime();}

  // Retrieve a product
  using Base::get;
  using Base::getByLabel;
  using Base::getByToken;
  using Base::getMany;
  using Base::getManyByType;
  using Base::getPointerByLabel;
  using Base::getValidHandle;

  // Retrieve a view to a collection of products
  using Base::getView;

  Run const& getRun() const;

  // Put a new product
  template <typename PROD> ProductID put(std::unique_ptr<PROD>&&);
  template <typename PROD> ProductID put(std::unique_ptr<PROD>&&, FullSemantic<Level::SubRun>);
  template <typename PROD> ProductID put(std::unique_ptr<PROD>&&, FragmentSemantic<Level::SubRun>);
  template <typename PROD> ProductID put(std::unique_ptr<PROD>&&, RangedFragmentSemantic<Level::SubRun>);

  // Put a new product with an instance name
  template <typename PROD> ProductID put(std::unique_ptr<PROD>&&, std::string const& instanceName);
  template <typename PROD> ProductID put(std::unique_ptr<PROD>&&, std::string const& instanceName, FullSemantic<Level::SubRun>);
  template <typename PROD> ProductID put(std::unique_ptr<PROD>&&, std::string const& instanceName, FragmentSemantic<Level::SubRun>);
  template <typename PROD> ProductID put(std::unique_ptr<PROD>&&, std::string const& instanceName, RangedFragmentSemantic<Level::SubRun>);

  // Expert-level
  using Base::removeCachedProduct;
  using Base::processHistory;

  EDProductGetter const*
  productGetter(ProductID const pid) const;

  template <typename T>
  using HandleT = Handle<T>;

private:

  // commit_() is called to complete the transaction represented by
  // this DataViewImpl. The friendships required are gross, but any
  // alternative is not great either.  Putting it into the
  // public interface is asking for trouble
  friend class InputSource;
  friend class DecrepitRelicInputSourceImplementation;
  friend class EDFilter;
  friend class EDProducer;

  void commit_(SubRunPrincipal&);

  ///Put a new product with a 'product instance name' and a 'range set'
  template <typename PROD>
  art::ProductID
  put_(std::unique_ptr<PROD>&& product,
       std::string const& productInstanceName,
       RangeSet const& rs);

  Principal const& principal_;
  SubRunAuxiliary const& aux_;
  std::unique_ptr<Run const> const run_;
  RangeSet productRangeSet_;
};

//================================================================
// Implementation

//----------------------------------------------------------------
// putting with no specified instance name

template <typename PROD>
art::ProductID
art::SubRun::put(std::unique_ptr<PROD>&& product)
{
  return put<PROD>(std::move(product), std::string{});
}

template <typename PROD>
art::ProductID
art::SubRun::put(std::unique_ptr<PROD>&& product,
                 FullSemantic<Level::SubRun> const semantic)
{
  return put<PROD>(std::move(product), std::string{}, semantic);
}

template <typename PROD>
art::ProductID
art::SubRun::put(std::unique_ptr<PROD>&& product,
                 FragmentSemantic<Level::SubRun> const semantic)
{
  return put<PROD>(std::move(product), std::string{}, semantic);
}

template <typename PROD>
art::ProductID
art::SubRun::put(std::unique_ptr<PROD>&& product,
                 RangedFragmentSemantic<Level::SubRun> semantic)
{
  return put<PROD>(std::move(product), std::string{}, std::move(semantic));
}

//----------------------------------------------------------------
// putting with specified instance name

template <typename PROD>
art::ProductID
art::SubRun::put(std::unique_ptr<PROD>&& product,
                 std::string const& productInstanceName)
{
  productRangeSet_.collapse();
  return put_<PROD>(std::move(product), productInstanceName, productRangeSet_);
}

template <typename PROD>
art::ProductID
art::SubRun::put(std::unique_ptr<PROD>&& product,
                 std::string const& productInstanceName,
                 FullSemantic<Level::SubRun>)
{
  return put_<PROD>(std::move(product), productInstanceName, RangeSet::forSubRun(id()));
}

template <typename PROD>
art::ProductID
art::SubRun::put(std::unique_ptr<PROD>&& product,
                 std::string const& productInstanceName,
                 FragmentSemantic<Level::SubRun>)
{
  static_assert(detail::CanBeAggregated<PROD>::value,
                "\n\n"
                "art error: A SubRun product put with the semantic 'SubRunFragment'\n"
                "           must be able to be aggregated. Please add the appropriate\n"
                "              void aggregate(T const&)\n"
                "           function to your class, or contact artists@fnal.gov.\n");

  if (productRangeSet_.collapse().is_full_subRun()) {
    throw art::Exception(art::errors::ProductPutFailure, "SubRun::put")
      << "\nCannot put a product corresponding to a full SubRun using\n"
      << "art::subRunFragment().  This can happen if you attempted to\n"
      << "put a product at beginSubRun using art::subRunFragment().\n"
      << "Please use either:\n"
      << "   art::fullSubRun(), or\n"
      << "   art::subRunFragment(art::RangeSet const&)\n"
      << "or contact artists@fnal.gov for assistance.\n";
  }
  return put_<PROD>(std::move(product), productInstanceName, productRangeSet_);
}

template <typename PROD>
art::ProductID
art::SubRun::put(std::unique_ptr<PROD>&& product,
                 std::string const& productInstanceName,
                 RangedFragmentSemantic<Level::SubRun> semantic)
{
  static_assert(detail::CanBeAggregated<PROD>::value,
                "\n\n"
                "art error: A SubRun product put with the semantic 'SubRunFragment'\n"
                "           must be able to be aggregated. Please add the appropriate\n"
                "              void aggregate(T const&)\n"
                "           function to your class, or contact artists@fnal.gov.\n");
  if (semantic.rs.collapse().is_full_subRun()) {
    throw Exception{errors::ProductPutFailure, "Run::put"}
      << "\nCannot put a product corresponding to a full SubRun using\n"
      << "art::subRunFragment(art::RangeSet&).  Please use:\n"
      << "   art::fullSubRun()\n"
      << "or contact artists@fnal.gov for assistance.\n";
  }
  return put_<PROD>(std::move(product), productInstanceName, semantic.rs);
}


template <typename PROD>
art::ProductID
art::SubRun::put_(std::unique_ptr<PROD>&& product,
                  std::string const& productInstanceName,
                  RangeSet const& rs)
{
  TypeID const tid{typeid(PROD)};
  if (product.get() == nullptr) {
    throw art::Exception{art::errors::NullPointerError, "SubRun::put"}
      << "\nA null unique_ptr was passed to 'put'.\n"
      << "The pointer is of type " << tid << ".\n"
      << "The specified productInstanceName was '" << productInstanceName << "'.\n";
  }

  if (!rs.is_valid()) {
    throw art::Exception{art::errors::ProductPutFailure, "SubRun::put"}
      << "\nCannot put a product with an invalid RangeSet.\n"
      << "Please contact artists@fnal.gov.\n";
  }

  auto const& pd = getProductDescription(tid, productInstanceName);
  auto wp = std::make_unique<Wrapper<PROD>>(std::move(product));

  auto result = putProducts().emplace(TypeLabel{tid, productInstanceName},
                                      PMValue{std::move(wp), pd, rs});
  if (!result.second) {
    throw art::Exception{art::errors::ProductPutFailure, "SubRun::put"}
      << "\nAttempt to put multiple products with the\n"
      << "following description onto the SubRun.\n"
      << "Products must be unique per SubRun.\n"
      << "=================================\n"
      << pd
      << "=================================\n";
  }

  return pd.productID();
}

#endif /* art_Framework_Principal_SubRun_h */

// Local Variables:
// mode: c++
// End:
