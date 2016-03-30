#ifndef art_Framework_Principal_NoDelayedReader_h
#define art_Framework_Principal_NoDelayedReader_h

#include "art/Persistency/Common/DelayedReader.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"

#include <memory>

class art::NoDelayedReader : public art::DelayedReader {
private:
  std::unique_ptr<EDProduct> getProduct_(BranchKey const& k, art::TypeID const &) const override;
};

#endif /* art_Framework_Principal_NoDelayedReader_h */

// Local Variables:
// mode: c++
// End:
