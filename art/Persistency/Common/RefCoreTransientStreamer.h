#ifndef art_Persistency_Common_RefCoreTransientStreamer_h
#define art_Persistency_Common_RefCoreTransientStreamer_h

#include "TClassStreamer.h"
#include "TClassRef.h"

class TBuffer;

namespace art {
  class EDProductGetter;
  class RefCoreTransientStreamer : public TClassStreamer {
  public:
    explicit RefCoreTransientStreamer(EDProductGetter const* ep) : prodGetter_(ep) {}

    void setProductGetter(EDProductGetter const* ep) {
      prodGetter_ = ep;
    }
    void operator() (TBuffer &R_b, void *objp);

  private:
    EDProductGetter const* prodGetter_;
  };

  void configureRefCoreTransientStreamer(EDProductGetter const* ep = 0);
}
#endif /* art_Persistency_Common_RefCoreTransientStreamer_h */

// Local Variables:
// mode: c++
// End:
