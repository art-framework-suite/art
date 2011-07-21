#include "art/Framework/Core/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProductList.h"
#include "art/Framework/Core/ProductMetaData.h"
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

std::vector<art::BranchDescription const*>
art::ProductMetaData::allBranchDescriptions() const
{
  return mpr_->allBranchDescriptions();
}

void
art::ProductMetaData::printBranchDescriptions(std::ostream& os) const
{
  typedef std::vector<BranchDescription const*> desc_vec;
  desc_vec descs = allBranchDescriptions();
  // Todo: replace this loop by the appropriate algorithm, probably cet::for_all.
  for (desc_vec::const_iterator i = descs.begin(), e = descs.end(); i !=e; ++i)
  {
    os << *i << "\n-----\n";
  }
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
