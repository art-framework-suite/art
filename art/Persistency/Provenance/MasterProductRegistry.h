#ifndef art_Persistency_Provenance_MasterProductRegistry_h
#define art_Persistency_Provenance_MasterProductRegistry_h
// vim: set sw=2:

#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/ProductList.h"
#include "cpp0x/array"
#include "cpp0x/memory"
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

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
  // Used for indices to find branch IDs by type and process.
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
  using PLUFunc = void(*)(void*, FileBlock const&);
  using ProductListUpdatedCallback = std::pair<PLUFunc, std::shared_ptr<void>>;
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
  std::vector<TypeLookup> const& productLookup() const {
    return productLookup_;
  }
  // Obtain lookup map to find a group by type of element
  // in a product which is a collection.
  std::vector<TypeLookup> const& elementLookup() const {
    return elementLookup_;
  }
  void print(std::ostream&) const;
  void addProduct(std::unique_ptr<BranchDescription>&&);
  void initFromFirstPrimaryFile(ProductList const&, FileBlock const&);
  void updateFromSecondaryFile(ProductList const&, FileBlock const&);
  std::string updateFromNewPrimaryFile(ProductList const&,
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
  // Support finding a BranchID by <product friendly class name, process name>.
  std::vector<TypeLookup> productLookup_;
  // Support finding a BranchID by
  // <product::value_type friendly class name, process name>.
  std::vector<TypeLookup> elementLookup_;
  std::vector<ProductListUpdatedCallback> productListUpdatedCallbacks_;
};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif // art_Persistency_Provenance_MasterProductRegistry_h
