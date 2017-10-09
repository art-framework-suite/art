#ifndef art_Framework_IO_ProductMix_MixOpBase_h
#define art_Framework_IO_ProductMix_MixOpBase_h

#include <string>

#include "art/Framework/IO/ProductMix/MixTypes.h"
#include "art/Framework/IO/Root/RootBranchInfoList.h"
#include "art/Utilities/fwd.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/Compatibility/BranchIDList.h"
#include "canvas/Persistency/Provenance/FileIndex.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "cetlib/exempt_ptr.h"

namespace art {
  class MixOpBase;

  class EDProduct;
  class Event;
  class PtrRemapper;
} // namespace art

class art::MixOpBase {
public:
  virtual ~MixOpBase() noexcept = default;

  virtual InputTag const& inputTag() const = 0;

  virtual TypeID const& inputType() const = 0;

  virtual std::string const& outputInstanceLabel() const = 0;

  virtual void mixAndPut(Event& e, PtrRemapper const& remap) const = 0;

  virtual void initializeBranchInfo(RootBranchInfoList const& rbiList) = 0;

  virtual ProductID incomingProductID() const = 0;

  virtual ProductID outgoingProductID() const = 0;

  virtual void readFromFile(
    EntryNumberSequence const& seq,
    cet::exempt_ptr<BranchIDLists const> branchIDLists) = 0;

  virtual BranchType branchType() const = 0;

protected:
  void configureStreamers(cet::exempt_ptr<BranchIDLists const> branchIDLists);
};
#endif /* art_Framework_IO_ProductMix_MixOpBase_h */

// Local Variables:
// mode: c++
// End:
