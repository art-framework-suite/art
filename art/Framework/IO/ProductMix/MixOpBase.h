#ifndef art_Framework_IO_ProductMix_MixOpBase_h
#define art_Framework_IO_ProductMix_MixOpBase_h

#include "art/Framework/IO/ProductMix/MixTypes.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Utilities/InputTag.h"

namespace art {
  class MixOpBase;

  class Event;
  class PtrRemapper;
} // namespace art

class art::MixOpBase {
public:
  virtual ~MixOpBase() noexcept = default;

  virtual TypeID inputType() const = 0;
  virtual InputTag const& inputTag() const = 0;
  virtual ProductID incomingProductID() const = 0;
  virtual ProductID outgoingProductID() const = 0;
  virtual BranchType branchType() const = 0;
  virtual EDProduct const* newIncomingWrappedProduct() const = 0;

  virtual void mixAndPut(Event& e,
                         SpecProdList const& incomingProducts,
                         PtrRemapper const& remap) const = 0;
  virtual void setIncomingProductID(ProductID) = 0;
};
#endif /* art_Framework_IO_ProductMix_MixOpBase_h */

// Local Variables:
// mode: c++
// End:
