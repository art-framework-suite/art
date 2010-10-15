#ifndef FWCore_Framework_NoDelayedReader_h
#define FWCore_Framework_NoDelayedReader_h

/*----------------------------------------------------------------------
----------------------------------------------------------------------*/

#include <memory>
#include "art/Framework/Core/DelayedReader.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"

namespace art {
  class NoDelayedReader : public DelayedReader {
  public:
    virtual ~NoDelayedReader();
  private:
    virtual std::auto_ptr<EDProduct> getProduct_(BranchKey const& k, EDProductGetter const* ep) const;
  };
}
#endif
