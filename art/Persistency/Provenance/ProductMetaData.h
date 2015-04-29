#ifndef art_Persistency_Provenance_ProductMetaData_h
#define art_Persistency_Provenance_ProductMetaData_h
// vim: set sw=2:

//
//  ProductMetaData
//
//  Singleton facade providing readonly access to the
//  MasterProductRegistry.
//

#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProductList.h"
#include "cetlib/exempt_ptr.h"
#include <ostream>
#include <vector>

// Used for testing
class MPRGlobalTestFixture;

namespace art {

class Schedule;

class ProductMetaData {

  friend class Schedule;

  // Used for testing.
  friend class ::MPRGlobalTestFixture;

public:

  typedef MasterProductRegistry::ProcessLookup ProcessLookup;
  typedef MasterProductRegistry::TypeLookup TypeLookup;
  using ProductListUpdatedCallback =
    MasterProductRegistry::ProductListUpdatedCallback;

public:

  // Give access to the only instance; throws if it is not yet made.
  static ProductMetaData const& instance()
  {
    if (!me) {
      throw Exception(errors::LogicError)
          << "ProductMetaData::instance called before the sole instance was created.";
    }
    return *me;
  }

private:

  // Only friends are permitted to create the instance.
  ProductMetaData(MasterProductRegistry const& mpr)
    : mpr_(&mpr)
  {
  }

  // Only the create_instance() member will create an instance.
  static void create_instance(MasterProductRegistry const& mpr)
  {
    if (me) {
      throw Exception(errors::LogicError)
          << "ProductMetaData::create_instance called more than once.";
    }
    me = new ProductMetaData(mpr);
  }

private:

  // Singleton pattern instance pointer.
  static ProductMetaData const* me;

public:

  ProductMetaData(ProductMetaData const&) = delete;
  ProductMetaData& operator=(ProductMetaData const&) = delete;

  // Accessors: this is the facade presented by ProductMetaData
  // for the MasterProductRegistry.
  ProductList const& productList() const
  {
    return mpr_->productList();
  }

  // Print all the BranchDescriptions we contain.
  void printBranchDescriptions(std::ostream& os) const
  {
    mpr_->print(os);
  }

  // Obtain lookup map to find a group by type of product.
  std::vector<TypeLookup> const& productLookup() const
  {
    return mpr_->productLookup();
  }

  // Obtain lookup map to find a group by type of element in a product
  // which is a collection.
  std::vector<TypeLookup> const& elementLookup() const
  {
    return mpr_->elementLookup();
  }

  // Return true if any product is produced in this process for
  // the given branch type.
  bool productProduced(BranchType which) const
  {
    return mpr_->productProduced(which);
  }

  std::vector<ProductListUpdatedCallback> const&
  productListUpdatedCallbacks() const {
    return mpr_->productListUpdatedCallbacks();
  }

private:

  cet::exempt_ptr<MasterProductRegistry const> mpr_;

};

inline
std::ostream&
operator<<(std::ostream& os, ProductMetaData const& pmd)
{
  pmd.printBranchDescriptions(os);
  return os;
}

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif // art_Persistency_Provenance_ProductMetaData_h
