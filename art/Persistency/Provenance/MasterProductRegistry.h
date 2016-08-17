#ifndef art_Persistency_Provenance_MasterProductRegistry_h
#define art_Persistency_Provenance_MasterProductRegistry_h
// vim: set sw=2:

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/DictionaryChecker.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "art/Persistency/Provenance/detail/type_aliases.h"

#include <array>
#include <iosfwd>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace Reflex {
  class Type;
}

namespace art {

  class BranchID;
  class BranchKey;
  class FileBlock;
  class MasterProductRegistry;

  std::ostream& operator<<(std::ostream& os, MasterProductRegistry const& mpr);

}

class art::MasterProductRegistry {

public:
  MasterProductRegistry(MasterProductRegistry const&) = delete;
  MasterProductRegistry& operator=(MasterProductRegistry const&) = delete;
  MasterProductRegistry();
  ProductList const& productList() const {
    return productList_;
  }
  ProductList::size_type size() const {
    return productList_.size();
  }
  bool productProduced(BranchType branchType) const {
    return productProduced_[branchType];
  }
  // Obtain lookup map to find a group by type of product.
  std::vector<BranchTypeLookup> const& productLookup() const {
    return productLookup_;
  }
  // Obtain lookup map to find a group by type of element
  // in a product which is a collection.
  std::vector<BranchTypeLookup> const& elementLookup() const {
    return elementLookup_;
  }
  void print(std::ostream&) const;
  void addProduct(std::unique_ptr<BranchDescription>&&);

  void initFromFirstPrimaryFile(ProductList const&, PerBranchTypePresence const&, FileBlock const&);
  void updateFromSecondaryFile (ProductList const&, PerBranchTypePresence const&, FileBlock const&);

  bool produced(BranchType const, BranchID const) const;

  static constexpr std::size_t DROPPED = std::numeric_limits<std::size_t>::max();

  std::size_t presentWithFileIdx(BranchType const, BranchID const) const;
  std::string updateFromNewPrimaryFile(ProductList const&,
                                       PerBranchTypePresence const&,
                                       std::string const& fileName,
                                       BranchDescription::MatchMode,
                                       FileBlock const&);

  void setFrozen();

  std::vector<ProductListUpdatedCallback> const&
  productListUpdatedCallbacks() const {
    return productListUpdatedCallbacks_;
  }

  void
  registerProductListUpdateCallback(ProductListUpdatedCallback cb) {
    productListUpdatedCallbacks_.push_back(cb);
  }

private:
  void checkDicts_(BranchDescription const & productDesc);

  ProductList productList_ {};
  bool frozen_ {false};
  std::array<bool, NumBranchTypes> productProduced_ {{false}}; //filled by aggregation
  std::vector<ProductList> perFileProds_ {};

  PerBranchTypePresence  perBranchPresenceLookup_ {{}};
  PerFilePresence perFilePresenceLookups_ {};
  // Support finding a BranchID by <product friendly class name, process name>.
  std::vector<BranchTypeLookup> productLookup_ {};
  // Support finding a BranchID by
  // <product::value_type friendly class name, process name>.
  std::vector<BranchTypeLookup> elementLookup_ {};
  std::vector<ProductListUpdatedCallback> productListUpdatedCallbacks_ {};

  DictionaryChecker dictChecker_ {};
};

// Local Variables:
// mode: c++
// End:
#endif /* art_Persistency_Provenance_MasterProductRegistry_h */
