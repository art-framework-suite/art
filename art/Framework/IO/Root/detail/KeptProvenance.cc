#include "art/Framework/IO/Root/detail/KeptProvenance.h"
#include "art/Framework/Principal/Principal.h"
#include "canvas/Persistency/Provenance/Parentage.h"

using namespace art;

detail::KeptProvenance::KeptProvenance(DropMetaData const dropMetaData,
                                       bool const dropMetaDataForDroppedData,
                                       std::set<ProductID>& branchesWithStoredHistory)
  : dropMetaData_{dropMetaData}
  , dropMetaDataForDroppedData_{dropMetaDataForDroppedData}
  , branchesWithStoredHistory_{branchesWithStoredHistory}
{}

ProductProvenance const&
detail::KeptProvenance::insert(ProductProvenance const& pp)
{
  return *provenance_.insert(pp).first;
}

ProductProvenance const&
detail::KeptProvenance::emplace(ProductID const pid, ProductStatus const status)
{
  return *provenance_.emplace(pid, status).first;
}


void
detail::KeptProvenance::insertAncestors(ProductProvenance const& iGetParents,
                                        Principal const& principal)
{
  if (dropMetaData_ == DropMetaData::DropAll) {
    return;
  }
  if (dropMetaDataForDroppedData_) {
    return;
  }
  auto const& parents = iGetParents.parentage().parents();
  for (auto const pid : parents) {
    branchesWithStoredHistory_.insert(pid);
    auto info = principal.branchMapper().branchToProductProvenance(pid);
    if (!info || dropMetaData_ != DropMetaData::DropNone) {
      continue;
    }
    auto const* bd = principal.getForOutput(info->productID(), false).desc();
    if (bd && bd->produced() && provenance_.insert(*info).second) {
      // FIXME: Remove recursion!
      insertAncestors(*info, principal);
    }
  }
}

void
detail::KeptProvenance::setStatus(ProductProvenance const& key,
                                  ProductStatus const status)
{
  if (provenance_.erase(key) != 1ull)
    throw Exception(errors::LogicError, "detail::KeptProvenance::setStatus")
      << "Attempt to set product status for product whose provenance is not being recorded.\n";
  provenance_.emplace(key.productID(), status);
}
