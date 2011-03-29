/*----------------------------------------------------------------------

BranchMapperWithReader:

----------------------------------------------------------------------*/


#include "art/Framework/IO/Input/BranchMapperWithReader.h"

#include "art/Persistency/Common/RefCoreStreamer.h"


namespace art {

  void
  BranchMapperWithReader<EventEntryInfo>::readProvenance_() const {
    setRefCoreStreamer(0, true);
    branchPtr_->SetAddress(&pInfoVector_);
    input::getEntry(branchPtr_, entryNumber_);
    BranchMapperWithReader<EventEntryInfo> * me = const_cast<BranchMapperWithReader<EventEntryInfo> *>(this);
    for (std::vector<EventEntryInfo>::const_iterator it = infoVector_.begin(), itEnd = infoVector_.end();
      it != itEnd; ++it) {
      me->insert(it->makeProductProvenance());
    }
  }

}  // art
