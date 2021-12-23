#ifndef art_Framework_Principal_Run_h
#define art_Framework_Principal_Run_h
// vim: set sw=2 expandtab :

//
//  This is the primary interface for accessing per run EDProducts
//  and inserting new derived products.
//
//  For its usage, see "art/Framework/Principal/DataViewImpl.h"
//

#include "art/Framework/Principal/DataViewImpl.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ProductSemantics.h"

namespace art {

  class Run final : private DataViewImpl {
  public:
    ~Run();

    explicit Run(RunPrincipal const& principal,
                 ModuleContext const& mc,
                 RangeSet const& rs = RangeSet::invalid());

    Run(Run const&) = delete;
    Run(Run&&) = delete;
    Run& operator=(Run const&) = delete;
    Run& operator=(Run&&) = delete;

    RunID id() const;
    RunNumber_t run() const;
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

    using DataViewImpl::getProductDescription;
    using DataViewImpl::getProductID;

    using DataViewImpl::getProcessParameterSet;
    using DataViewImpl::productGetter;
    using DataViewImpl::removeCachedProduct;

    // Obsolete interface (will be deprecated)
    using DataViewImpl::get;
    using DataViewImpl::getByLabel;

    using DataViewImpl::movePutProductsToPrincipal;

    // Product insertion
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  std::string const& instance = {});
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  FullSemantic<Level::Run> semantic);
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  FragmentSemantic<Level::Run> semantic);
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  RangedFragmentSemantic<Level::Run> semantic);
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  std::string const& instance,
                  FullSemantic<Level::Run>);
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  std::string const& instance,
                  FragmentSemantic<Level::Run>);
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  std::string const& instance,
                  RangedFragmentSemantic<Level::Run> semantic);

  private:
    RunPrincipal const& runPrincipal_;
    // The RangeSet to be used by any products put by the user.
    // Cannot be const because we call collapse() on it.
    RangeSet rangeSet_;
  };

  template <typename PROD>
  ProductID
  Run::put(std::unique_ptr<PROD>&& edp, std::string const& instance)
  {
    // Should be protected when a Run is shared among threads.
    return DataViewImpl::put(move(edp), instance, rangeSet_.collapse());
  }

  template <typename PROD>
  ProductID
  Run::put(std::unique_ptr<PROD>&& edp, FullSemantic<Level::Run> const semantic)
  {
    return put(move(edp), "", semantic);
  }

  template <typename PROD>
  ProductID
  Run::put(std::unique_ptr<PROD>&& edp,
           FragmentSemantic<Level::Run> const semantic)
  {
    return put(move(edp), "", semantic);
  }

  template <typename PROD>
  ProductID
  Run::put(std::unique_ptr<PROD>&& edp,
           RangedFragmentSemantic<Level::Run> semantic)
  {
    return put(move(edp), "", std::move(semantic));
  }

  template <typename PROD>
  ProductID
  Run::put(std::unique_ptr<PROD>&& edp,
           std::string const& instance,
           FullSemantic<Level::Run>)
  {
    return DataViewImpl::put(move(edp), instance, RangeSet::forRun(id()));
  }

  template <typename PROD>
  ProductID
  Run::put(std::unique_ptr<PROD>&& edp,
           std::string const& instance,
           FragmentSemantic<Level::Run>)
  {
    static_assert(
      detail::CanBeAggregated<PROD>::value,
      "\n\n"
      "art error: A Run product put with the semantic 'RunFragment'\n"
      "           must be able to be aggregated. Please add the appropriate\n"
      "              void aggregate(T const&)\n"
      "           function to your class, or contact artists@fnal.gov.\n");
    if (rangeSet_.collapse().is_full_run()) {
      throw Exception{errors::ProductPutFailure, "Run::put"}
        << "\nCannot put a product corresponding to a full Run using\n"
        << "art::runFragment().  This can happen if you attempted to\n"
        << "put a product at beginRun using art::runFragment().\n"
        << "Please use either:\n"
        << "   art::fullRun(), or\n"
        << "   art::runFragment(art::RangeSet const&)\n"
        << "or contact artists@fnal.gov for assistance.\n";
    }
    return DataViewImpl::put(move(edp), instance, rangeSet_);
  }

  template <typename PROD>
  ProductID
  Run::put(std::unique_ptr<PROD>&& edp,
           std::string const& instance,
           RangedFragmentSemantic<Level::Run> semantic)
  {
    static_assert(
      detail::CanBeAggregated<PROD>::value,
      "\n\n"
      "art error: A Run product put with the semantic 'RunFragment'\n"
      "           must be able to be aggregated. Please add the appropriate\n"
      "              void aggregate(T const&)\n"
      "           function to your class, or contact artists@fnal.gov.\n");
    if (semantic.rs.collapse().is_full_run()) {
      throw Exception{errors::ProductPutFailure, "Run::put"}
        << "\nCannot put a product corresponding to a full Run using\n"
        << "art::runFragment(art::RangeSet&).  Please use:\n"
        << "   art::fullRun()\n"
        << "or contact artists@fnal.gov for assistance.\n";
    }
    if (!semantic.rs.is_valid()) {
      throw Exception{errors::ProductPutFailure, "Run::put"}
        << "\nCannot put a product with an invalid RangeSet.\n"
        << "Please contact artists@fnal.gov.\n";
    }
    return DataViewImpl::put(move(edp), instance, semantic.rs);
  }

} // namespace art

#endif /* art_Framework_Principal_Run_h */

// Local Variables:
// mode: c++
// End:
