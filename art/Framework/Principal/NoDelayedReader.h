#ifndef art_Framework_Principal_NoDelayedReader_h
#define art_Framework_Principal_NoDelayedReader_h

#include "art/Framework/Principal/DelayedReader.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"
#include "cpp0x/memory"

namespace art {

  class NoDelayedReader
    : public DelayedReader
  {
  public:
    virtual ~NoDelayedReader();
  private:
    virtual std::auto_ptr<EDProduct> getProduct_(BranchKey const& k, EDProductGetter const* ep) const;
  };

}

#endif /* art_Framework_Principal_NoDelayedReader_h */

// Local Variables:
// mode: c++
// End:
