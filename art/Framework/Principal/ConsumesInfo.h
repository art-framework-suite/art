#ifndef art_Framework_Principal_ConsumesInfo_h
#define art_Framework_Principal_ConsumesInfo_h
// vim: set sw=2 expandtab :

// FIXME: THESE NOTES SHOULD GO SOMEWHERE ELSE
//
//============================================================================
// Consumer is the base class for all module types that retrieve
// products.  See below for guidance as to which interface should be
// called for a given context.
//
// N.B. All 'consumes*' or 'mayConsume*' calls should be done in a
//      module's constructor, before processing any beginJob
//      callbacks.
//
// Unconditional retrieval of products
// -----------------------------------
//
// For products that are always intended to be retrieved for a given
// module execution, the 'consumes' family of calls should be invoked
// in a user's module constructor (e.g.):
//
//   consumes<int>(input_tag);     // => ProductToken<int>
//   consumesMany<int>();          // => void
//   consumesView<int>(input_tag); // => ViewToken<int>
//
// Making such calls tells the framework that the particular module in
// question will make the following product retrievals (e.g.):
//
//   e.getValidHandle<int>(input_tag);
//   e.getManyByType<int>();
//   art::View<int> v;
//   e.getView(input_tag, v);
//
// The returned type of the consumes calls are shown above.  To
// eventually facilitate faster product lookup, the returned tokens
// can be supplied by the user (e.g.):
//
//   ProductToken<int> intToken_; // data member of module class
//   e.getValidHandle(intToken_); // => ValidHandle<int>
//
//   ViewToken<int> viewToken_; // data member of module class
//   std::vector<int const*> v;
//   e.getView(viewToken_, v);
//
// The consumesMany function template does not return a token.
//
// Conditional retrieval of products
// ---------------------------------
//
// If there are situations in which a product *may* be retrieved
// (e.g. within an 'if' block), then the *mayConsume* interface should
// be considered instead of the consumes interface:
//
//   mayConsume<int>(input_tag); // => ProductToken<int>
//   mayConsumeMany<int>();      // => void
//   mayConsumeView<int>();      // => ViewToken<int>
//
// The return types of the functions are the same as their 'consumes'
// partners for unconditional product retrieving.  However, how the
// tokens are used by the framework may be different than how they are
// used in the 'consumes' case.
//
// Retrieving products in non-module contexts
// ------------------------------------------
//
// There are cases where products are retrieved in non-module
// contexts.  Although such product retrieval is not forbidden, it is
// not a generally recommended usage pattern.  The 'consumes'
// interface is, therefore, not supported in non-module contexts.
//============================================================================

#include "canvas/Persistency/Provenance/BranchType.h"

#include <array>
#include <atomic>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace art {

  class ModuleDescription;
  class ProductInfo;

  class ConsumesInfo {
  public: // MEMBER FUNCTIONS -- Special Member Functions
    ~ConsumesInfo();
    ConsumesInfo(ConsumesInfo const&) = delete;
    ConsumesInfo(ConsumesInfo&&) = delete;
    ConsumesInfo& operator=(ConsumesInfo const&) = delete;
    ConsumesInfo& operator=(ConsumesInfo&&) = delete;

  public:
    static ConsumesInfo* instance();
    static std::string assemble_consumes_statement(BranchType const,
                                                   ProductInfo const&);
    static std::string module_context(ModuleDescription const&);

    void setRequireConsumes(bool const);

    // Maps module label to run, per-branch consumes info.
    using consumables_t =
      std::map<std::string const,
               std::array<std::vector<ProductInfo>, NumBranchTypes>>;

    consumables_t::mapped_type const&
    consumables(std::string const& module_label) const
    {
      return consumables_.at(module_label);
    }

    void collectConsumes(std::string const& module_label,
                         consumables_t::mapped_type const& consumables);

    // This is used by get*() in ProductRetriever.
    void validateConsumedProduct(BranchType const,
                                 ModuleDescription const&,
                                 ProductInfo const& productInfo);

    void showMissingConsumes() const;

  private:
    ConsumesInfo();

    // Protects access to consumables_ and missingConsumes_.
    mutable std::recursive_mutex mutex_{};

    std::atomic<bool> requireConsumes_;

    // Maps module label to run, per-branch consumes info.  Note that
    // there is only one entry per module label.  This is intentional
    // so that we do not need to store consumes information for each
    // replicated module object.
    consumables_t consumables_;

    // Maps module label to run, per-branch missing product consumes info.
    std::map<std::string const,
             std::array<std::set<ProductInfo>, NumBranchTypes>>
      missingConsumes_;
  };
} // namespace art

#endif /* art_Framework_Principal_ConsumesInfo_h */

// Local Variables:
// mode: c++
// End:
