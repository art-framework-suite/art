#ifndef art_Persistency_Provenance_MasterProductRegistry_h
#define art_Persistency_Provenance_MasterProductRegistry_h
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/type_aliases.h"

#include <array>
#include <functional>
#include <iosfwd>
#include <memory>
#include <string>

namespace art {

  using ProductLists = std::array<ProductDescriptionsByID, NumBranchTypes>;
  using ProductListUpdatedCallback = std::function<void(ProductTables const&)>;

  class MasterProductRegistry {
  public:

    explicit MasterProductRegistry() = default;
    MasterProductRegistry(MasterProductRegistry const&) = delete;

    void registerProductListUpdatedCallback(ProductListUpdatedCallback cb);

    void finalizeForProcessing(ProductTables const&);
    void addProductsFromModule(ProductDescriptions&&);
    void updateFromModule(std::unique_ptr<ProductTables>&&);
    void updateFromInputFile(ProductTables const&);

    auto const& productTables() const { return productTables_; }

    void print(std::ostream&) const;

    bool productProduced(BranchType branchType) const { return productProduced_[branchType]; }

  private:

    void addProduct_(BranchDescription&&);
    void updateProductTables_(ProductDescriptionsByID const& pl);
    void updateProductTables_(ProductTables const& pl);

    bool allowExplicitRegistration_{true};

    // Data members initialized once per process:
    ProductTables productTables_{ProductTables::invalid()};
    std::array<bool, NumBranchTypes> productProduced_{{false}}; //filled by aggregation
    std::vector<ProductListUpdatedCallback> productListUpdatedCallbacks_{};
  };

  std::ostream&
  operator<<(std::ostream&, MasterProductRegistry const&);

} // namespace art

#endif /* art_Persistency_Provenance_MasterProductRegistry_h */

// Local Variables:
// mode: c++
// End:
