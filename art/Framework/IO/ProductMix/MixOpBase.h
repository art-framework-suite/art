#ifndef art_Framework_IO_ProductMix_MixOpBase_h
#define art_Framework_IO_ProductMix_MixOpBase_h

#include <string>

#include "art/Persistency/Provenance/FileIndex.h"
#include "art/Framework/IO/Root/RootBranchInfoList.h"
#include "art/Framework/IO/ProductMix/MixContainerTypes.h"

namespace art {
  class MixOpBase;

  class EDProduct;
  class Event;
  class InputTag;
  class PtrRemapper;
  class TypeID;
}

class art::MixOpBase {
public:
  virtual
  InputTag const &inputTag() const = 0;

  virtual
  TypeID const &inputType() const = 0;

  virtual
  std::string const &outputInstanceLabel() const = 0;

  virtual
  void
  mixAndPut(Event &e,
            PtrRemapper const &remap) const = 0;

  virtual
  void
  initializeBranchInfo(RootBranchInfoList const &rbiList) = 0;

  virtual
  BranchID
  incomingBranchID() const = 0;

  virtual
  BranchID
  outgoingBranchID() const = 0;

  virtual
  void
  readFromFile(EntryNumberSequence const &seq) = 0;
};
#endif /* art_Framework_IO_ProductMix_MixOpBase_h */

// Local Variables:
// mode: c++
// End:
