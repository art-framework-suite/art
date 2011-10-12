#ifndef art_Persistency_Common_CacheStreamers_h
#define art_Persistency_Common_CacheStreamers_h

#include "TClassStreamer.h"
#include "TClassRef.h"
class TBuffer;

namespace art {
  class ConstPtrCacheStreamer : public TClassStreamer {
  public:
    explicit ConstPtrCacheStreamer() : cl_("art::ConstPtrCache") {}

    void operator()(TBuffer & R_b, void * objp);

  private:
    TClassRef cl_;
  };

  class BoolCacheStreamer : public TClassStreamer {
  public:
    explicit BoolCacheStreamer() : cl_("art::BoolCache") {}

    void operator()(TBuffer & R_b, void * objp);

  private:
    TClassRef cl_;
  };


  void setCacheStreamers();
}

#endif /* art_Persistency_Common_CacheStreamers_h */

// Local Variables:
// mode: c++
// End:
