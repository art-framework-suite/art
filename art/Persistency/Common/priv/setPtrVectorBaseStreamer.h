#ifndef art_Persistency_Common_priv_setPtrVectorBaseStreamer_h
#define art_Persistency_Common_priv_setPtrVectorBaseStreamer_h

namespace art {
  namespace priv {
    class PtrVectorBaseStreamer;
    void setPtrVectorBaseStreamer();
  }
}

#include "TClassStreamer.h"

class TBuffer;

class art::priv::PtrVectorBaseStreamer : public TClassStreamer {
public:
  void operator()(TBuffer &R_b, void *objp);
};

#endif /* art_Persistency_Common_priv_setPtrVectorBaseStreamer_h */

// Local Variables:
// mode: c++
// End:
