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

    static Results make(ResultsPrincipal& p, ModuleContext const& mc);
    static Results make(ResultsPrincipal const& p, ModuleContext const& mc);

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

    using ProductRetriever::getProductDescription;
    using ProductRetriever::getProductID;
    using ProductRetriever::productGetter;

    // Obsolete interface (will be deprecated)
    using ProductRetriever::get;
    using ProductRetriever::getByLabel;

    template <typename PROD>
    ProductID
    put(std::unique_ptr<PROD>&& edp, std::string const& instance = {})
    {
      assert(inserter_);
      return inserter_->put(move(edp), instance);
    }

  private:
    void commitProducts();

    // Give access to commitProducts(...).
    friend class ResultsProducer;

    explicit Results(ResultsPrincipal const& p,
                     ModuleContext const& mc,
                     std::optional<ProductInserter> inserter);

    std::optional<ProductInserter> inserter_;
  };

} // namespace art

#endif /* art_Framework_Principal_Results_h */

// Local Variables:
// mode: c++
// End:
