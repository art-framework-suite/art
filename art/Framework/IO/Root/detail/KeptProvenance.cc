#include "art/Framework/IO/Root/detail/KeptProvenance.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Principal.h"
#include "canvas/Persistency/Provenance/Parentage.h"

#include <set>
#include <vector>

using namespace std;

namespace art {
namespace detail {

KeptProvenance::
~KeptProvenance()
{
}

KeptProvenance::
KeptProvenance(DropMetaData const dropMetaData, bool const dropMetaDataForDroppedData,
               set<ProductID>& branchesWithStoredHistory)
  : dropMetaData_{dropMetaData}
  , dropMetaDataForDroppedData_{dropMetaDataForDroppedData}
  , branchesWithStoredHistory_{branchesWithStoredHistory}
{
}

ProductProvenance const&
KeptProvenance::
insert(ProductProvenance const& pp)
{
  return *provenance_.insert(pp).first;
}

ProductProvenance const&
KeptProvenance::
emplace(ProductID const pid, ProductStatus const status)
{
  return *provenance_.emplace(pid, status).first;
}

void
KeptProvenance::
insertAncestors(ProductProvenance const& pp, Principal const& principal)
{
  vector<ProductProvenance const*> stacked_pp;
  stacked_pp.push_back(&pp);
  while (1) {
    if (stacked_pp.size() == 0) {
      break;
    }
    ProductProvenance const* current_pp = stacked_pp.back();
    stacked_pp.pop_back();
    for (auto const parent_pid : current_pp->parentage().parents()) {
      branchesWithStoredHistory_.insert(parent_pid);
      auto parent_pp = principal.branchMapper().branchToProductProvenance(parent_pid);
      if (!parent_pp || (dropMetaData_ != DropMetaData::DropNone)) {
        continue;
      }
      auto const* parent_bd = principal.getForOutput(parent_pp->productID(), false).desc();
      if (!parent_bd) {
        // FIXME: Is this an error condition?
        continue;
      }
      if (!parent_bd->produced()) {
        // We got it from the input, nothing to do.
        continue;
      }
      if (!provenance_.insert(*parent_pp).second) {
        // Already there, done.
        continue;
      }
      if ((dropMetaData_ != DropMetaData::DropAll) && !dropMetaDataForDroppedData_) {
        stacked_pp.push_back(parent_pp.get());
      }
    }
  }
}

void
KeptProvenance::
setStatus(ProductProvenance const& key, ProductStatus const status)
{
  if (provenance_.erase(key) != 1ull) {
    throw Exception(errors::LogicError, "KeptProvenance::setStatus")
        << "Attempt to set product status for product whose provenance is not being recorded.\n";
  }
  provenance_.emplace(key.productID(), status);
}

} // namespace detail
} // namespace art

