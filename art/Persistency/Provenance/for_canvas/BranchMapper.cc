// ======================================================================
//
// BranchMapper: Manages the per event/subRun/run per product provenance.
//
// ======================================================================

#include "canvas/Persistency/Provenance/BranchMapper.h"

using art::BranchMapper;

BranchMapper::BranchMapper(bool delayedRead) :
  entryInfoSet_(),
  delayedRead_(delayedRead)
{ }

void
BranchMapper::readProvenance() const
{
  if (delayedRead_) {
    delayedRead_ = false;
    readProvenance_();
  }
}

BranchMapper::result_t
BranchMapper::insert(std::unique_ptr<ProductProvenance const> && pp_ptr)
{
  readProvenance();
  result_t result(pp_ptr.get());
  entryInfoSet_[ result->branchID() ].reset(pp_ptr.release());
  return result;
}

BranchMapper::result_t
BranchMapper::branchToProductProvenance(BranchID const &bid) const
{
  readProvenance();
  eiSet::const_iterator it = entryInfoSet_.find(bid);
  return it == entryInfoSet_.end() ? result_t()
         : result_t(it->second.get());
}

// ======================================================================
