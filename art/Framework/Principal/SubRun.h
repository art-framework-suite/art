#ifndef art_Framework_Principal_SubRun_h
#define art_Framework_Principal_SubRun_h
// vim: set sw=2 expandtab :

//
//  This is the primary interface for accessing per subRun
//  EDProducts and inserting new derived per subRun EDProducts.
//
//  For its usage, see "art/Framework/Principal/DataViewImpl.h"
//

#include "art/Framework/Principal/DataViewImpl.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ProductSemantics.h"
#include "canvas/Persistency/Provenance/SubRunID.h"

#include <memory>

namespace art {

  class SubRun final : private DataViewImpl {
  public:
    ~SubRun();

    explicit SubRun(SubRunPrincipal const& srp,
                    ModuleContext const& mc,
                    RangeSet const& rs = RangeSet::invalid());

    SubRun(SubRun const&) = delete;
    SubRun(SubRun&&) = delete;
    SubRun& operator=(SubRun const&) = delete;
    SubRun& operator=(SubRun&&) = delete;

    SubRunID id() const;
    RunNumber_t run() const;
    SubRunNumber_t subRun() const;
    Timestamp const& beginTime() const;
    Timestamp const& endTime() const;
    ProcessHistory const& processHistory() const;

    using DataViewImpl::getHandle;
    using DataViewImpl::getInputTags;
    using DataViewImpl::getMany;
    using DataViewImpl::getProduct;
    using DataViewImpl::getProductTokens;
    using DataViewImpl::getValidHandle;
    using DataViewImpl::getView;

    Run const& getRun() const;
    using DataViewImpl::getProductDescription;
    using DataViewImpl::getProductID;

    using DataViewImpl::productGetter;
    using DataViewImpl::removeCachedProduct;

    // Obsolete interface (will be deprecated)
    using DataViewImpl::get;
    using DataViewImpl::getByLabel;

    using DataViewImpl::movePutProductsToPrincipal;

    // Product insertion - subrun
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  std::string const& instance = {});
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  FullSemantic<Level::SubRun> semantic);
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  FragmentSemantic<Level::SubRun> semantic);
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  RangedFragmentSemantic<Level::SubRun> semantic);
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  std::string const& instance,
                  FullSemantic<Level::SubRun>);
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  std::string const& instance,
                  FragmentSemantic<Level::SubRun>);
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  std::string const& instance,
                  RangedFragmentSemantic<Level::SubRun> semantic);

  private:
    SubRunPrincipal const& subRunPrincipal_;
    std::unique_ptr<Run const> const run_;
    // The RangeSet to be used by any products put by the user.
    // Cannot be const because we call collapse() on it.
    RangeSet rangeSet_;
  };

  template <typename PROD>
  ProductID
  SubRun::put(std::unique_ptr<PROD>&& edp, std::string const& instance)
  {
    // Should be protected when a SubRun is shared among threads.
    return DataViewImpl::put(move(edp), instance, rangeSet_.collapse());
  }

  template <typename PROD>
  ProductID
  SubRun::put(std::unique_ptr<PROD>&& edp,
              FullSemantic<Level::SubRun> const semantic)
  {
    return put(move(edp), "", semantic);
  }

  template <typename PROD>
  ProductID
  SubRun::put(std::unique_ptr<PROD>&& edp,
              FragmentSemantic<Level::SubRun> const semantic)
  {
    return put(move(edp), "", semantic);
  }

  template <typename PROD>
  ProductID
  SubRun::put(std::unique_ptr<PROD>&& edp,
              RangedFragmentSemantic<Level::SubRun> semantic)
  {
    return put(move(edp), "", std::move(semantic));
  }

  template <typename PROD>
  ProductID
  SubRun::put(std::unique_ptr<PROD>&& edp,
              std::string const& instance,
              FullSemantic<Level::SubRun>)
  {
    return DataViewImpl::put(move(edp), instance, RangeSet::forSubRun(id()));
  }

  template <typename PROD>
  ProductID
  SubRun::put(std::unique_ptr<PROD>&& edp,
              std::string const& instance,
              FragmentSemantic<Level::SubRun>)
  {
    static_assert(
      detail::CanBeAggregated<PROD>::value,
      "\n\n"
      "art error: A SubRun product put with the semantic 'SubRunFragment'\n"
      "           must be able to be aggregated. Please add the appropriate\n"
      "              void aggregate(T const&)\n"
      "           function to your class, or contact artists@fnal.gov.\n");
    if (rangeSet_.collapse().is_full_subRun()) {
      throw Exception(errors::ProductPutFailure, "SubRun::put")
        << "\nCannot put a product corresponding to a full SubRun using\n"
        << "art::subRunFragment().  This can happen if you attempted to\n"
        << "put a product at beginSubRun using art::subRunFragment().\n"
        << "Please use either:\n"
        << "   art::fullSubRun(), or\n"
        << "   art::subRunFragment(art::RangeSet const&)\n"
        << "or contact artists@fnal.gov for assistance.\n";
    }
    return DataViewImpl::put(move(edp), instance, rangeSet_);
  }

  template <typename PROD>
  ProductID
  SubRun::put(std::unique_ptr<PROD>&& edp,
              std::string const& instance,
              RangedFragmentSemantic<Level::SubRun> semantic)
  {
    static_assert(
      detail::CanBeAggregated<PROD>::value,
      "\n\n"
      "art error: A SubRun product put with the semantic 'SubRunFragment'\n"
      "           must be able to be aggregated. Please add the appropriate\n"
      "              void aggregate(T const&)\n"
      "           function to your class, or contact artists@fnal.gov.\n");
    if (semantic.rs.collapse().is_full_subRun()) {
      throw Exception{errors::ProductPutFailure, "SubRun::put"}
        << "\nCannot put a product corresponding to a full SubRun using\n"
        << "art::subRunFragment(art::RangeSet&).  Please use:\n"
        << "   art::fullSubRun()\n"
        << "or contact artists@fnal.gov for assistance.\n";
    }
    if (!semantic.rs.is_valid()) {
      throw Exception{errors::ProductPutFailure, "SubRun::put"}
        << "\nCannot put a product with an invalid RangeSet.\n"
        << "Please contact artists@fnal.gov.\n";
    }
    return DataViewImpl::put(move(edp), instance, semantic.rs);
  }

} // namespace art

#endif /* art_Framework_Principal_SubRun_h */

// Local Variables:
// mode: c++
// End:
