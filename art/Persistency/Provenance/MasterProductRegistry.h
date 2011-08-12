#ifndef art_Persistency_Provenance_MasterProductRegistry_h
#define art_Persistency_Provenance_MasterProductRegistry_h
////////////////////////////////////////////////////////////////////////
// MasterProductRegistry
//
// The One True Repository of product metadata.
////////////////////////////////////////////////////////////////////////

#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/ProductList.h"
#include "cpp0x/array"
#include "cpp0x/memory"

#include "Reflex/Type.h"

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

namespace art {
  class MasterProductRegistry;

  class BranchID;
  class BranchKey;

  std::ostream &operator<<(std::ostream &os, MasterProductRegistry const &mpr);
}

class art::MasterProductRegistry {
public:
  MasterProductRegistry();

  // Used for indices to find branch IDs by type and process.
  // TODO: Consider moving this typedef outside of the class. ProductMetaData needs to
  // share this typedef with its users, and so has to include this header to get the typedef.
  // If we move the typedef to a different header, then MasterProductRegistry could be moved
  // to the "detail" directory.
  typedef std::map<std::string, std::vector<BranchID> > ProcessLookup;
  typedef std::map<std::string, ProcessLookup> TypeLookup;

  ////////////////////////////////////////////////////////////////////////
  // const functions available to clients of ProductMetaData.
  std::vector<std::string> allBranchNames() const;

  ProductList const &productList() const;
  ProductList::size_type size() const;

  bool anyProducts(BranchType bracnhType) const;
  bool productProduced(BranchType branchType) const;

  // Obtain lookup map to find a group by type of product.
  TypeLookup const &productLookup() const;
  // Obtain lookup map to find a group by type of element in a product which is a collection.
  TypeLookup const &elementLookup() const;

  void print(std::ostream& os) const;

  ////////////////////////////////////////////////////////////////////////
  // Mutators for use while we are unfrozen only.
  void addProduct(std::auto_ptr<BranchDescription> bdp);

  void updateFromInput(ProductList const &other);

  // Mutator for use while frozen only.
  std::string merge(ProductList const &other,
                    std::string const &fileName,
                    BranchDescription::MatchMode m);

  // Freeze.
  void setFrozen();

private:
  void copyProduct(BranchDescription const &productDesc);
  void fillElementLookup(Reflex::Type const &type,
                         BranchID const &id,
                         BranchKey const &bk);
  void throwIfNotFrozen() const;
  void throwIfFrozen() const;
  void processFrozenProductList();

  // Member data.
  ProductList productList_;
  bool frozen_;
  std::array<bool, NumBranchTypes> productProduced_;
  TypeLookup productLookup_;
  TypeLookup elementLookup_;
};

inline
art::ProductList const &
art::MasterProductRegistry::productList() const {
  return productList_;
}

inline
art::ProductList::size_type
art::MasterProductRegistry::size() const {
  return productList_.size();
}

inline
bool
art::MasterProductRegistry::productProduced(BranchType branchType) const {
  return productProduced_[branchType];
}

inline
art::MasterProductRegistry::TypeLookup const &
art::MasterProductRegistry::productLookup() const {
  return productLookup_;
}

inline
art::MasterProductRegistry::TypeLookup const &
art::MasterProductRegistry::elementLookup() const {
  return elementLookup_;
}
#endif /* art_Persistency_Provenance_MasterProductRegistry_h */

// Local Variables:
// mode: c++
// End:
