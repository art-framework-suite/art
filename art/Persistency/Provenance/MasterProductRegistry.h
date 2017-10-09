#ifndef art_Persistency_Provenance_MasterProductRegistry_h
#define art_Persistency_Provenance_MasterProductRegistry_h
// vim: set sw=2:

//====================================================================
// MasterProductRegistry
//
// Although this is not technically a registry, it is the
// representation for ProductMetaData, which is the read-only facade
// to the single static instance of this class within art.  As such,
// this class should be treated as a registry.
//
// All modifications to the registry happen either at system startup,
// or when input files are opened.  Once we intend to support
// concurrent reading from different input files, it will be necessary
// for any registry modifications to happen in a thread-safe manner.
//
// This class is not altogether necessary--it is meant to facilitate
// more efficient lookup of products.  Steps should be taken to
// determine how this class, and the associated ProductMetaData class,
// could be removed in favor of alternative efficiency improvements in
// product lookup that do not rely on a registry.
// ====================================================================

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "canvas/Persistency/Provenance/type_aliases.h"

#include <array>
#include <functional>
#include <iosfwd>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace art {

  class MasterProductRegistry;
  using ProductListUpdatedCallback = std::function<void(ProductList const&)>;

  std::ostream& operator<<(std::ostream& os, MasterProductRegistry const& mpr);
}

class art::MasterProductRegistry {
public:
  explicit MasterProductRegistry() = default;

  MasterProductRegistry(MasterProductRegistry const&) = delete;
  MasterProductRegistry& operator=(MasterProductRegistry const&) = delete;

  void registerProductListUpdatedCallback(ProductListUpdatedCallback cb);

  void finalizeForProcessing();
  void addProductsFromModule(ProductDescriptions&&);
  void updateFromModule(std::unique_ptr<ProductList>&&);
  void updateFromInputFile(ProductList const&);

  auto const&
  productList() const
  {
    return productList_;
  }
  auto
  size() const
  {
    return productList_.size();
  }

  void print(std::ostream&) const;

  bool
  productProduced(BranchType branchType) const
  {
    return productProduced_[branchType];
  }

private:
  void addProduct_(BranchDescription&&);
  void updateProductLists_(ProductList const& pl);

  bool allowExplicitRegistration_{true};

  // Data members initialized once per process:
  ProductList productList_{};
  std::array<bool, NumBranchTypes> productProduced_{
    {false}}; // filled by aggregation
  std::vector<ProductListUpdatedCallback> productListUpdatedCallbacks_{};
};

// Local Variables:
// mode: c++
// End:
#endif /* art_Persistency_Provenance_MasterProductRegistry_h */
