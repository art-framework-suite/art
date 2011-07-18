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

// ----------------------------------------------------------------------

class TBranch;

namespace art {

  class BranchMapperWithReader : public BranchMapper {
  public:
    BranchMapperWithReader(TBranch * branch, input::EntryNumber entryNumber);

    virtual ~BranchMapperWithReader() {}

  private:
    virtual void readProvenance_() const;

    TBranch * branchPtr_;
    input::EntryNumber entryNumber_;
  };  // BranchMapperWithReader

  inline
  BranchMapperWithReader::BranchMapperWithReader(TBranch * branch, input::EntryNumber entryNumber) :
         BranchMapper(true),
         branchPtr_(branch), entryNumber_(entryNumber)
  { }

  inline
  void
  BranchMapperWithReader::readProvenance_() const {
    std::vector<ProductProvenance> infoVector;
    std::vector<ProductProvenance> * pInfoVector(&infoVector);
    branchPtr_->SetAddress(&pInfoVector);
    input::getEntry(branchPtr_, entryNumber_);
    BranchMapperWithReader * me = const_cast<BranchMapperWithReader*>(this);
    for (typename std::vector<ProductProvenance>::const_iterator it = infoVector.begin(), itEnd = infoVector.end();
      it != itEnd; ++it) {
      me->insert(*it);
    }
  }

}  // art

// ======================================================================

#endif /* art_Framework_IO_Root_BranchMapperWithReader_h */

// Local Variables:
// mode: c++
// End:
