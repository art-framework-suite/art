#include "art/Framework/IO/Root/detail/KeptProvenance.h"
#include "art/Framework/Principal/Principal.h"
#include "canvas/Persistency/Provenance/Parentage.h"

using namespace art;

detail::KeptProvenance::KeptProvenance(
  DropMetaData const dropMetaData,
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
    auto info = principal.branchMapper().branchToProductProvenance(pid);
    if (!info || dropMetaData_ != DropMetaData::DropNone) {
      continue;
    }
    // These two data structures should be in sync.
    branchesWithStoredHistory_.insert(pid);
    (void)provenance_.insert(*info).second;

    auto const* pd = principal.getForOutput(info->productID(), false).desc();
    if (pd && pd->produced()) {
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
      << "Attempt to set product status for product whose provenance is not "
         "being recorded.\n";
  provenance_.emplace(key.productID(), status);
}
