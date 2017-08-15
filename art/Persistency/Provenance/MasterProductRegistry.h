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
#include "art/Persistency/Provenance/detail/type_aliases.h"

#include <array>
#include <iosfwd>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace art {

  class ProductID;
  class BranchKey;
  class FileBlock;
  class MasterProductRegistry;

  std::ostream& operator<<(std::ostream& os, MasterProductRegistry const& mpr);
}

class art::MasterProductRegistry {
public:

  static constexpr std::size_t DROPPED {std::numeric_limits<std::size_t>::max()};

  explicit MasterProductRegistry() = default;

  MasterProductRegistry(MasterProductRegistry const&) = delete;
  MasterProductRegistry& operator=(MasterProductRegistry const&) = delete;

  void addProduct(std::unique_ptr<BranchDescription>&&);
  void registerProductListUpdatedCallback(ProductListUpdatedCallback cb);

  void finalizeForProcessing();
  void updateFromModule(std::unique_ptr<ProductList>&&);
  void updateFromPrimaryFile(ProductList const&,
                             PerBranchTypePresence const&,
                             FileBlock const&);
  void updateFromSecondaryFile(ProductList const&,
                               PerBranchTypePresence const&,
                               FileBlock const&);

  auto const& productList() const { return productList_; }
  auto size() const { return productList_.size(); }

  void print(std::ostream&) const;

  bool productProduced(BranchType branchType) const {
    return productProduced_[branchType];
  }
  bool produced(BranchType, ProductID) const;
  std::size_t presentWithFileIdx(BranchType, ProductID) const;

private:

  void updateProductLists_(ProductList const& pl);

  bool allowExplicitRegistration_{true};

  // Data members initialized once per process:
  ProductList productList_{};
  std::array<bool, NumBranchTypes> productProduced_{{false}}; //filled by aggregation
  std::vector<ProductListUpdatedCallback> productListUpdatedCallbacks_{};

  // Data members reset per primary input file:
  PerBranchTypePresence  perBranchPresenceLookup_{{}}; // Fill with 1 empty list
  PerFilePresence perFilePresenceLookups_{};
  // Support finding a ProductID by <product friendly class name, process name>.
  //    std::vector<BranchTypeLookup> productLookup_;
  // Support finding a ProductID by
  // <product::value_type friendly class name, process name>.
  //    std::vector<BranchTypeLookup> elementLookup_;
};

// Local Variables:
// mode: c++
// End:
#endif /* art_Persistency_Provenance_MasterProductRegistry_h */
