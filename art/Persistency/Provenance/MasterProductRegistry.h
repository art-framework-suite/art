#ifndef art_Persistency_Provenance_MasterProductRegistry_h
#define art_Persistency_Provenance_MasterProductRegistry_h
// vim: set sw=2:

#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/ProductList.h"
#include "cpp0x/array"
#include "cpp0x/memory"
#include <iosfwd>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <unordered_set>

namespace Reflex {
class Type;
} // namespace Reflex

namespace art {

class BranchID;
class BranchKey;
class FileBlock;
class MasterProductRegistry;

//
//typedef std::map<BranchKey, BranchDescription> ProductList;
//

std::ostream& operator<<(std::ostream& os, MasterProductRegistry const& mpr);

class MasterProductRegistry {
public:
  // Used for indices to find branch IDs by branchType, class type and process.
  // TODO: Consider moving this typedef outside of the class.
  // ProductMetaData needs to share this typedef with its users,
  // and so has to include this header to get the typedef.
  // If we move the typedef to a different header, then
  // MasterProductRegistry could be moved to the "detail" directory.
  //--
  // The key is the process name.
  typedef std::map<std::string const, std::vector<BranchID>> ProcessLookup;
  // The key is the friendly class name.
  typedef std::map<std::string const, ProcessLookup> TypeLookup;
  typedef std::array<TypeLookup, NumBranchTypes> BranchTypeLookup;
  using ProductListUpdatedCallback = std::function<void (FileBlock const &)>;
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

  using PresenceSet           = std::set<art::BranchID>;
  using PerBranchTypePresence = std::array<PresenceSet,NumBranchTypes>;
  using PerFilePresence       = std::vector<PerBranchTypePresence>;

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
  ProductList productList_;
  bool frozen_;
  std::array<bool, NumBranchTypes> productProduced_;
  std::vector<ProductList> perFileProds_;

  PerBranchTypePresence producedPresenceLookup_;
  PerFilePresence perFilePresenceLookups_;
  // Support finding a BranchID by <product friendly class name, process name>.
  std::vector<BranchTypeLookup> productLookup_;
  // Support finding a BranchID by
  // <product::value_type friendly class name, process name>.
  std::vector<BranchTypeLookup> elementLookup_;
  std::vector<ProductListUpdatedCallback> productListUpdatedCallbacks_;
};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif // art_Persistency_Provenance_MasterProductRegistry_h
