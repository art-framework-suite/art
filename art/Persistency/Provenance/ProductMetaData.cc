#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProductList.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Utilities/Exception.h"

namespace art
{
  class BranchDescription;
}

art::ProductMetaData const* art::ProductMetaData::me = 0;

art::ProductMetaData const&
art::ProductMetaData::instance()
{
  if (!ProductMetaData::me)
    {
      throw art::Exception(errors::LogicError)
      << "ProductMetaData::instance called before the sole instance was created.";
    }
  return *me;
}

void
art::ProductMetaData::create_instance(MasterProductRegistry const& mpr)
{
  if (ProductMetaData::me)
  {
    throw art::Exception(errors::LogicError)
      << "ProductMetaData::create_instance called more than once.";
  }
  ProductMetaData::me = new ProductMetaData(mpr);
}

art::ProductMetaData::ProductMetaData(MasterProductRegistry const& mpr) :
  mpr_(&mpr)
{ }

art::ProductList const&
art::ProductMetaData::productList() const
{
  return mpr_->productList();
}

 void
 art::ProductMetaData::printBranchDescriptions(std::ostream& os) const
 {
   mpr_->print(os);
 }

art::ProductMetaData::TypeLookup const&
art::ProductMetaData::productLookup() const
{
  return mpr_->productLookup();
}

art::ProductMetaData::TypeLookup const&
art::ProductMetaData::elementLookup() const
{
  return mpr_->elementLookup();
}

bool
art::ProductMetaData::productProduced(BranchType which) const
{
  return mpr_->productProduced(which);
}
