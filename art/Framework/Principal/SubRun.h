#ifndef art_Framework_Principal_SubRun_h
#define art_Framework_Principal_SubRun_h
// vim: set sw=2 expandtab :

//
//  This is the primary interface for accessing per subRun
//  EDProducts and inserting new derived per subRun EDProducts.
//
//  For its usage, see "art/Framework/Principal/ProductRetriever.h"
//

#include "art/Framework/Principal/ProductRetriever.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ProductSemantics.h"
#include "canvas/Persistency/Provenance/SubRunID.h"

#include <cassert>
#include <memory>
#include <optional>

#define PUT_DEPRECATION_MSG                                                    \
  "\n\n"                                                                       \
  "art warning: The SubRun::put(...) function without a product semantic is "  \
  "deprecated.\n"                                                              \
  "             Please adjust your usage to include the correct semantic "     \
  "(e.g.):\n\n"                                                                \
  "               subrun.put(std::move(product), art::fullSubRun());\n"        \
  "               subrun.put(std::move(product), instanceName, "               \
  "art::subRunFragment());\n\n"                                                \
  "             Generally, SubRun::put calls made in beginSubRun should "      \
  "include art::fullSubRun();\n"                                               \
  "             SubRun::put calls made in endSubRun should use "               \
  "art::subRunFragment().\n"                                                   \
  "             Contact artists@fnal.gov for assistance or details.\n\n"

namespace art {

  class SubRun final : private ProductRetriever {
  public:
    ~SubRun();

    explicit SubRun(SubRunPrincipal const& srp,
                    ModuleContext const& mc,
                    std::optional<ProductInserter> inserter = std::nullopt,
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

    using ProductRetriever::getHandle;
    using ProductRetriever::getInputTags;
    using ProductRetriever::getMany;
    using ProductRetriever::getProduct;
    using ProductRetriever::getProductTokens;
    using ProductRetriever::getValidHandle;
    using ProductRetriever::getView;

    Run const& getRun() const;
    using ProductRetriever::getProcessParameterSet;
    using ProductRetriever::getProductDescription;
    using ProductRetriever::getProductProvenance;

    using ProductRetriever::getProductID;
    using ProductRetriever::productGetter;

    // Obsolete interface (will be deprecated)
    using ProductRetriever::get;
    using ProductRetriever::getByLabel;

    // Product insertion - subrun
    template <typename PROD>
    [[deprecated(PUT_DEPRECATION_MSG)]] PutHandle<PROD> put(
      std::unique_ptr<PROD>&& edp,
      std::string const& instance = {});
    template <typename PROD>
    PutHandle<PROD> put(std::unique_ptr<PROD>&& edp,
                        FullSemantic<Level::SubRun> semantic);
    template <typename PROD>
    PutHandle<PROD> put(std::unique_ptr<PROD>&& edp,
                        FragmentSemantic<Level::SubRun> semantic);
    template <typename PROD>
    PutHandle<PROD> put(std::unique_ptr<PROD>&& edp,
                        RangedFragmentSemantic<Level::SubRun> semantic);
    template <typename PROD>
    PutHandle<PROD> put(std::unique_ptr<PROD>&& edp,
                        std::string const& instance,
                        FullSemantic<Level::SubRun>);
    template <typename PROD>
    PutHandle<PROD> put(std::unique_ptr<PROD>&& edp,
                        std::string const& instance,
                        FragmentSemantic<Level::SubRun>);
    template <typename PROD>
    PutHandle<PROD> put(std::unique_ptr<PROD>&& edp,
                        std::string const& instance,
                        RangedFragmentSemantic<Level::SubRun> semantic);

  private:
    void commitProducts();

    // Give access to commitProducts(...).
    friend class detail::Analyzer;
    friend class detail::Filter;
    friend class detail::Producer;
    friend class ProducingService;

    std::optional<ProductInserter> inserter_;
    SubRunPrincipal const& subRunPrincipal_;
    Run const run_;
    // The RangeSet to be used by any products put by the user.
    // Cannot be const because we call collapse() on it.
    RangeSet rangeSet_;
  };

  template <typename PROD>
  PutHandle<PROD>
  SubRun::put(std::unique_ptr<PROD>&& edp, std::string const& instance)
  {
    assert(inserter_);
    // Should be protected when a SubRun is shared among threads.
    return inserter_->put(std::move(edp), instance, rangeSet_.collapse());
  }

  template <typename PROD>
  PutHandle<PROD>
  SubRun::put(std::unique_ptr<PROD>&& edp,
              FullSemantic<Level::SubRun> const semantic)
  {
    return put(std::move(edp), "", semantic);
  }

  template <typename PROD>
  PutHandle<PROD>
  SubRun::put(std::unique_ptr<PROD>&& edp,
              FragmentSemantic<Level::SubRun> const semantic)
  {
    return put(std::move(edp), "", semantic);
  }

  template <typename PROD>
  PutHandle<PROD>
  SubRun::put(std::unique_ptr<PROD>&& edp,
              RangedFragmentSemantic<Level::SubRun> semantic)
  {
    return put(std::move(edp), "", std::move(semantic));
  }

  template <typename PROD>
  PutHandle<PROD>
  SubRun::put(std::unique_ptr<PROD>&& edp,
              std::string const& instance,
              FullSemantic<Level::SubRun>)
  {
    assert(inserter_);
    return inserter_->put(std::move(edp), instance, RangeSet::forSubRun(id()));
  }

  template <typename PROD>
  PutHandle<PROD>
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
    assert(inserter_);
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
    return inserter_->put(std::move(edp), instance, rangeSet_);
  }

  template <typename PROD>
  PutHandle<PROD>
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
    assert(inserter_);
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
    return inserter_->put(std::move(edp), instance, semantic.rs);
  }

} // namespace art

#undef PUT_DEPRECATION_MSG

#endif /* art_Framework_Principal_SubRun_h */

// Local Variables:
// mode: c++
// End:
