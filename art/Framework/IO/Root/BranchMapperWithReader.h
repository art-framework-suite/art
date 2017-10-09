#ifndef art_Framework_IO_Root_BranchMapperWithReader_h
#define art_Framework_IO_Root_BranchMapperWithReader_h

// ======================================================================
//
// BranchMapperWithReader
//
// ======================================================================

#include "TBranch.h"
#include "art/Framework/IO/Root/Inputfwd.h"
#include "canvas/Persistency/Provenance/BranchMapper.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include <vector>

class TBranch;

namespace art {
  class BranchMapperWithReader;
}

// ----------------------------------------------------------------------

class art::BranchMapperWithReader : public BranchMapper {
public:
  BranchMapperWithReader(TBranch* branch, input::EntryNumber entryNumber);

private:
  void readProvenance_() const override;

  TBranch* branchPtr_;
  input::EntryNumber entryNumber_;

}; // BranchMapperWithReader

inline art::BranchMapperWithReader::BranchMapperWithReader(
  TBranch* branch,
  input::EntryNumber entryNumber)
  : BranchMapper(true), branchPtr_(branch), entryNumber_(entryNumber)
{}

inline void
art::BranchMapperWithReader::readProvenance_() const
{
  typedef std::vector<ProductProvenance> ppVec;

  ppVec infoVector;
  ppVec* pInfoVector(&infoVector);

  branchPtr_->SetAddress(&pInfoVector);
  input::getEntry(branchPtr_, entryNumber_);

  auto me = const_cast<BranchMapperWithReader*>(this);
  for (auto const& info : infoVector) {
    me->insert(std::make_unique<ProductProvenance const>(info));
  }
}

  // ======================================================================

#endif /* art_Framework_IO_Root_BranchMapperWithReader_h */

// Local Variables:
// mode: c++
// End:
