#ifndef art_Persistency_Provenance_MasterProductRegistry_h
#define art_Persistency_Provenance_MasterProductRegistry_h
// vim: set sw=2 expandtab :

#include <functional>
#include <vector>

namespace art {

  class ProductTables;

  using ProductListUpdatedCallback = std::function<void(ProductTables const&)>;

  class MasterProductRegistry {
  public:

    explicit MasterProductRegistry() = default;
    MasterProductRegistry(MasterProductRegistry const&) = delete;

    void registerProductListUpdatedCallback(ProductListUpdatedCallback cb);

    void finalizeForProcessing(ProductTables const&);
    void updateFromInputFile(ProductTables const&);

  private:
    std::vector<ProductListUpdatedCallback> productListUpdatedCallbacks_{};
  };

} // namespace art

#endif /* art_Persistency_Provenance_MasterProductRegistry_h */

// Local Variables:
// mode: c++
// End:
