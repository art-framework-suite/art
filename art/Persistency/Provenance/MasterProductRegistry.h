#ifndef art_Persistency_Provenance_MasterProductRegistry_h
#define art_Persistency_Provenance_MasterProductRegistry_h
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/DictionaryChecker.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "art/Persistency/Provenance/detail/type_aliases.h"

#include <cstddef>
#include <array>
#include <iosfwd>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace art {

class MasterProductRegistry {

public:

  static constexpr std::size_t DROPPED{std::numeric_limits<std::size_t>::max()};

public:

  explicit MasterProductRegistry() = default;

  MasterProductRegistry(MasterProductRegistry const&) = delete;

  MasterProductRegistry&
  operator=(MasterProductRegistry const&) = delete;

public:

  void
  addProduct(std::unique_ptr<BranchDescription>&&);

  void
  registerProductListUpdatedCallback(std::function<void()>);

  void
  finalizeForProcessing();

  void
  updateFromPrimaryFile(std::map<BranchKey, BranchDescription> const&, std::array<std::unordered_set<ProductID, ProductID::Hash>, NumBranchTypes> const&);

  void
  updateFromSecondaryFile(std::map<BranchKey, BranchDescription> const&, std::array<std::unordered_set<ProductID, ProductID::Hash>, NumBranchTypes> const&);

  auto const&
  productList() const
  {
    return productList_;
  }

  std::size_t
  size() const
  {
    return productList_.size();
  }

  // Obtain lookup map to find a group by type of product.
  std::vector<std::array<std::map<std::string const, std::map<std::string const, std::vector<ProductID>>>, NumBranchTypes>> const&
  productLookup() const
  {
    return productLookup_;
  }

  // Obtain lookup map to find a group by type of element in a product
  // which is a collection.
  std::vector<std::array<std::map<std::string const, std::map<std::string const, std::vector<ProductID>>>, NumBranchTypes>> const&
  elementLookup() const
  {
    return elementLookup_;
  }

  void
  print(std::ostream&) const;

  bool
  productProduced(BranchType branchType) const
  {
    return productProduced_[branchType];
  }

  bool
  produced(BranchType, ProductID) const;

  std::size_t
  presentWithFileIdx(BranchType, ProductID) const;

private:

  void
  setPresenceLookups_(std::map<BranchKey, BranchDescription> const& pl, PerBranchTypePresence const& presList);

  void
  updateProductLists_(std::map<BranchKey, BranchDescription> const& pl);

  void
  checkDicts_(BranchDescription const& productDesc);

  bool
  allowExplicitRegistration_{true};

private:

  std::map<BranchKey, BranchDescription>
  productList_{};

  std::array<bool, NumBranchTypes>
  productProduced_{{false}};

  std::vector<std::function<void()>>
  productListUpdatedCallbacks_{};

  DictionaryChecker
  dictChecker_{};

  std::vector<std::map<BranchKey, BranchDescription>>
  perFileProds_{{}};

  std::array<std::unordered_set<ProductID, ProductID::Hash>, NumBranchTypes>
  perBranchPresenceLookup_{{}};

  std::vector<std::array<std::unordered_set<ProductID, ProductID::Hash>, NumBranchTypes>>
  perFilePresenceLookups_{};

  std::vector<std::array<std::map<std::string const, std::map<std::string const, std::vector<ProductID>>>, NumBranchTypes>>
  productLookup_;

  std::vector<std::array<std::map<std::string const, std::map<std::string const, std::vector<ProductID>>>, NumBranchTypes>>
  elementLookup_;

};

std::ostream&
operator<<(std::ostream&, MasterProductRegistry const&);

} // namespace art

#endif /* art_Persistency_Provenance_MasterProductRegistry_h */

// Local Variables:
// mode: c++
// End:
