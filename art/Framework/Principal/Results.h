#ifndef art_Framework_Principal_Results_h
#define art_Framework_Principal_Results_h
// vim: set sw=2 expandtab :

// ==================================================================
//  This is the primary interface for accessing results-level
//  EDProducts and inserting new results-level EDProducts.
//
//  For its usage, see "art/Framework/Principal/ProductRetriever.h"
// ==================================================================

#include "art/Framework/Principal/ProductInserter.h"
#include "art/Framework/Principal/ProductRetriever.h"
#include "art/Framework/Principal/fwd.h"

#include <optional>

namespace art {

  class Results final : private ProductRetriever {
  public:
    ~Results();

    explicit Results(ResultsPrincipal const& p,
                     ModuleContext const& mc,
                     std::optional<ProductInserter> inserter = std::nullopt);

    Results(Results const&) = delete;
    Results(Results&&) = delete;
    Results& operator=(Results const&) = delete;
    Results& operator=(Results&&) = delete;

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

    template <typename PROD>
    PutHandle<PROD>
    put(std::unique_ptr<PROD>&& edp, std::string const& instance = {})
    {
      assert(inserter_);
      return inserter_->put(move(edp), instance);
    }

  private:
    void commitProducts();

    // Give access to commitProducts(...).
    friend class ResultsProducer;

    std::optional<ProductInserter> inserter_;
  };

} // namespace art

#endif /* art_Framework_Principal_Results_h */

// Local Variables:
// mode: c++
// End:
