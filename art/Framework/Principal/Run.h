#ifndef art_Framework_Principal_Run_h
#define art_Framework_Principal_Run_h
// vim: set sw=2 expandtab :

//
//  This is the primary interface for accessing per run EDProducts
//  and inserting new derived products.
//
//  For its usage, see "art/Framework/Principal/ProductRetriever.h"
//

#include "art/Framework/Principal/ProductInserter.h"
#include "art/Framework/Principal/ProductRetriever.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ProductSemantics.h"

#include <cassert>
#include <optional>

#define PUT_DEPRECATION_MSG                                                    \
  "\n\n"                                                                       \
  "art warning: The Run::put(...) function without a product semantic is "     \
  "deprecated.\n"                                                              \
  "             Please adjust your usage to include the correct semantic "     \
  "(e.g.):\n\n"                                                                \
  "               run.put(move(product), art::fullRun());\n"                   \
  "               run.put(move(product), instanceName, "                       \
  "art::runFragment());\n\n"                                                   \
  "             Generally, Run::put calls made in beginRun should include "    \
  "art::fullRun();\n"                                                          \
  "             Run::put calls made in endRun should use "                     \
  "art::runFragment().\n"                                                      \
  "             Contact artists@fnal.gov for assistance or details.\n\n"

namespace art {

  class Run final : private ProductRetriever {
  public:
    ~Run();

    explicit Run(RunPrincipal const& srp,
                 ModuleContext const& mc,
                 std::optional<ProductInserter> inserter = std::nullopt,
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

    using ProductRetriever::getHandle;
    using ProductRetriever::getInputTags;
    using ProductRetriever::getMany;
    using ProductRetriever::getProduct;
    using ProductRetriever::getProductTokens;
    using ProductRetriever::getValidHandle;
    using ProductRetriever::getView;

    using ProductRetriever::getProcessParameterSet;
    using ProductRetriever::getProductDescription;
    using ProductRetriever::getProductProvenance;

    using ProductRetriever::getProductID;
    using ProductRetriever::productGetter;

    // Obsolete interface (will be deprecated)
    using ProductRetriever::get;
    using ProductRetriever::getByLabel;

    // Product insertion
    template <typename PROD>
    [[deprecated(PUT_DEPRECATION_MSG)]] PutHandle<PROD> put(
      std::unique_ptr<PROD>&& edp,
      std::string const& instance = {});
    template <typename PROD>
    PutHandle<PROD> put(std::unique_ptr<PROD>&& edp,
                        FullSemantic<Level::Run> semantic);
    template <typename PROD>
    PutHandle<PROD> put(std::unique_ptr<PROD>&& edp,
                        FragmentSemantic<Level::Run> semantic);
    template <typename PROD>
    PutHandle<PROD> put(std::unique_ptr<PROD>&& edp,
                        RangedFragmentSemantic<Level::Run> semantic);
    template <typename PROD>
    PutHandle<PROD> put(std::unique_ptr<PROD>&& edp,
                        std::string const& instance,
                        FullSemantic<Level::Run>);
    template <typename PROD>
    PutHandle<PROD> put(std::unique_ptr<PROD>&& edp,
                        std::string const& instance,
                        FragmentSemantic<Level::Run>);
    template <typename PROD>
    PutHandle<PROD> put(std::unique_ptr<PROD>&& edp,
                        std::string const& instance,
                        RangedFragmentSemantic<Level::Run> semantic);

  private:
    void commitProducts();

    // Give access to commitProducts(...).
    friend class detail::Analyzer;
    friend class detail::Filter;
    friend class detail::Producer;
    friend class ProducingService;

    std::optional<ProductInserter> inserter_;
    RunPrincipal const& runPrincipal_;
    // The RangeSet to be used by any products put by the user.
    // Cannot be const because we call collapse() on it.
    RangeSet rangeSet_;
  };

  template <typename PROD>
  PutHandle<PROD>
  Run::put(std::unique_ptr<PROD>&& edp, std::string const& instance)
  {
    // Should be protected when a Run is shared among threads.
    assert(inserter_);
    return inserter_->put(move(edp), instance, rangeSet_.collapse());
  }

  template <typename PROD>
  PutHandle<PROD>
  Run::put(std::unique_ptr<PROD>&& edp, FullSemantic<Level::Run> const semantic)
  {
    return put(move(edp), "", semantic);
  }

  template <typename PROD>
  PutHandle<PROD>
  Run::put(std::unique_ptr<PROD>&& edp,
           FragmentSemantic<Level::Run> const semantic)
  {
    return put(move(edp), "", semantic);
  }

  template <typename PROD>
  PutHandle<PROD>
  Run::put(std::unique_ptr<PROD>&& edp,
           RangedFragmentSemantic<Level::Run> semantic)
  {
    return put(move(edp), "", std::move(semantic));
  }

  template <typename PROD>
  PutHandle<PROD>
  Run::put(std::unique_ptr<PROD>&& edp,
           std::string const& instance,
           FullSemantic<Level::Run>)
  {
    assert(inserter_);
    return inserter_->put(move(edp), instance, RangeSet::forRun(id()));
  }

  template <typename PROD>
  PutHandle<PROD>
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
    assert(inserter_);
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
    return inserter_->put(move(edp), instance, rangeSet_);
  }

  template <typename PROD>
  PutHandle<PROD>
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
    assert(inserter_);
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
    return inserter_->put(move(edp), instance, semantic.rs);
  }

} // namespace art

#undef PUT_DEPRECATION_MSG

#endif /* art_Framework_Principal_Run_h */

// Local Variables:
// mode: c++
// End:
