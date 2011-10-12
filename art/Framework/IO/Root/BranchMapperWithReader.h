#ifndef art_Framework_IO_Root_BranchMapperWithReader_h
#define art_Framework_IO_Root_BranchMapperWithReader_h

// ======================================================================
//
// BranchMapperWithReader
//
// ======================================================================

#include "TBranch.h"
#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include <vector>

class TBranch;

namespace art {
  class BranchMapperWithReader;
}

// ----------------------------------------------------------------------

class art::BranchMapperWithReader
  : public BranchMapper
{
public:
  BranchMapperWithReader(TBranch * branch, input::EntryNumber entryNumber);

  virtual ~BranchMapperWithReader() {}

private:
  virtual void readProvenance_() const;

  TBranch * branchPtr_;
  input::EntryNumber entryNumber_;

};  // BranchMapperWithReader

inline
art::BranchMapperWithReader::BranchMapperWithReader(TBranch * branch, input::EntryNumber entryNumber) :
  BranchMapper(true),
  branchPtr_  (branch),
  entryNumber_(entryNumber)
{ }

inline
void
art::BranchMapperWithReader::readProvenance_() const
{
  typedef  std::vector<ProductProvenance>  ppVec;
  typedef  typename ppVec::const_iterator  iter_t;

  ppVec infoVector;
  ppVec * pInfoVector(&infoVector);

  branchPtr_->SetAddress(&pInfoVector);
  input::getEntry(branchPtr_, entryNumber_);

  BranchMapperWithReader * me = const_cast<BranchMapperWithReader*>(this);
  for (iter_t it  = infoVector.begin()
            , end = infoVector.end(); it != end; ++it) {
    std::auto_ptr<ProductProvenance const> ap( new ProductProvenance(*it) );
    me->insert(ap);
  }
}

// ======================================================================

#endif /* art_Framework_IO_Root_BranchMapperWithReader_h */

// Local Variables:
// mode: c++
// End:
