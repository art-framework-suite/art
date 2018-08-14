#ifndef art_Framework_IO_ProductMix_MixIOPolicy_h
#define art_Framework_IO_ProductMix_MixIOPolicy_h

////////////////////////////////////////////////////////////////////////
// MixIOPolicy
//
// Fill in.
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/IO/ProductMix/MixOpBase.h"
#include "canvas/Persistency/Provenance/Compatibility/BranchIDList.h"
#include "canvas/Persistency/Provenance/FileIndex.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/exempt_ptr.h"

#include <string>

namespace art {

  using MixOpList = std::vector<std::unique_ptr<MixOpBase>>;

  class MixIOPolicy {
  public:
    virtual ~MixIOPolicy() noexcept = default;

    virtual EventAuxiliarySequence generateEventAuxiliarySequence(
      EntryNumberSequence const&) = 0;
    virtual bool fileOpen() const = 0;
    virtual std::size_t nEventsInFile() const = 0;
    virtual FileIndex const& fileIndex() const = 0;
    virtual cet::exempt_ptr<BranchIDLists const> branchIDLists() const = 0;
    virtual void openAndReadMetaData(std::string fileName,
                                     MixOpList& mixOps) = 0;
    virtual SpecProdList readFromFile(MixOpBase const& mixOp,
                                      EntryNumberSequence const& seq) = 0;
  };
}
#endif /* art_Framework_IO_ProductMix_MixIOPolicy_h */

// Local Variables:
// mode: c++
// End:
