#ifndef art_Framework_IO_ProductMix_MixOpBase_h
#define art_Framework_IO_ProductMix_MixOpBase_h

#include <cstddef>

#include "art/Persistency/Provenance/FileIndex.h"
#include "art/Framework/IO/Root/RootBranchInfoList.h"

namespace art {
  class MixOpBase;

  class EDProduct;
  class Event;
  class InputTag;
  class PtrRemapper;
  class TypeID;
}

class TTree;

class art::MixOpBase {
public:
  typedef std::map<FileIndex::EntryNumber_t, EventID> EventIDIndex;
  typedef std::vector<EventID> EventIDSequence;
  typedef std::vector<FileIndex::EntryNumber_t> EntryNumberSequence;

  virtual
  InputTag const &inputTag() const = 0;

  virtual
  TypeID const &inputType() const = 0;

  virtual
  std::string const &outputInstanceLabel() const = 0;

  virtual
  void
  mixAndPut(Event &e,
            PtrRemapper const &remap,
            EventIDSequence const &seq) const = 0;

  virtual
  BranchID
  incomingBranchID(RootBranchInfoList const &rbiList) const = 0;

  virtual
  BranchID
  outgoingBranchID() const = 0;

  virtual
  void
  readFromFile(TTree *eventTree,
               EntryNumberSequence const &seq) = 0;
};
#endif /* art_Framework_IO_ProductMix_MixOpBase_h */

// Local Variables:
// mode: c++
// End:
