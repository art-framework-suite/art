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
#include "canvas/Persistency/Provenance/DictionaryChecker.h"
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
  void setFrozen();
  void initFromFirstPrimaryFile(ProductList const&, PerBranchTypePresence const&, FileBlock const&);
  std::string updateFromNewPrimaryFile(ProductList const&,
                                       PerBranchTypePresence const&,
                                       std::string const& fileName,
                                       FileBlock const&);
  void updateFromSecondaryFile(ProductList const&, PerBranchTypePresence const&, FileBlock const&);
  void registerProductListUpdatedCallback(ProductListUpdatedCallback cb);

  auto const& productList() const { return productList_; }
  auto size() const { return productList_.size(); }

  // Obtain lookup map to find a group by type of product.
  auto const& productLookup() const { return productLookup_; }

  // Obtain lookup map to find a group by type of element in a product
  // which is a collection.
  auto const& elementLookup() const { return elementLookup_; }

  void print(std::ostream&) const;

  bool productProduced(BranchType branchType) const {
    return productProduced_[branchType];
  }
  bool produced(BranchType, ProductID) const;
  std::size_t presentWithFileIdx(BranchType, ProductID) const;

private:
  void resetPresenceFlags_(ProductList const& pl,
                           PerBranchTypePresence const& presList);
  void checkDicts_(BranchDescription const & productDesc);

  ProductList productList_ {};
  bool frozen_ {false};
  std::array<bool, NumBranchTypes> productProduced_ {{false}}; //filled by aggregation
  std::vector<ProductList> perFileProds_ {{}}; // Fill with 1 empty list

  PerBranchTypePresence  perBranchPresenceLookup_ {{}};
  PerFilePresence perFilePresenceLookups_ {};
  // Support finding a ProductID by <product friendly class name, process name>.
  std::vector<BranchTypeLookup> productLookup_ {};
  // Support finding a ProductID by
  // <product::value_type friendly class name, process name>.
  std::vector<BranchTypeLookup> elementLookup_ {};
  std::vector<ProductListUpdatedCallback> productListUpdatedCallbacks_ {};

  DictionaryChecker dictChecker_ {};
};

// Local Variables:
// mode: c++
// End:
#endif /* art_Persistency_Provenance_MasterProductRegistry_h */
