#ifndef DataFormats_Common_RefCoreStreamer_h
#define DataFormats_Common_RefCoreStreamer_h

#include "TClassStreamer.h"
#include "TClassRef.h"
#include <assert.h>

class TBuffer;

namespace art {
  class EDProductGetter;
  class RefCoreStreamer : public TClassStreamer {
  public:
    explicit RefCoreStreamer(EDProductGetter const* ep) : cl_("art::RefCore"), prodGetter_(ep) {}

    void setProductGetter(EDProductGetter const* ep) {
	prodGetter_ = ep;
    }
    void operator() (TBuffer &R_b, void *objp);

  private:
    TClassRef cl_;
    EDProductGetter const* prodGetter_;
  };

  class RefCoreTransientStreamer : public TClassStreamer {
  public:
    explicit RefCoreTransientStreamer(EDProductGetter const* ep) : cl_("art::RefCore::RefCoreTransients"), prodGetter_(ep) {}

    void setProductGetter(EDProductGetter const* ep) {
	prodGetter_ = ep;
    }
    void operator() (TBuffer &R_b, void *objp);

  private:
    TClassRef cl_;
    EDProductGetter const* prodGetter_;
  };

  class ProductIDStreamer : public TClassStreamer {
  public:
    explicit ProductIDStreamer(EDProductGetter const* ep) : cl_("art::ProductID"), prodGetter_(ep) {}

    void setProductGetter(EDProductGetter const* ep) {
	prodGetter_ = ep;
    }
    void operator() (TBuffer &R_b, void *objp);

  private:
    TClassRef cl_;
    EDProductGetter const* prodGetter_;
  };

  void setRefCoreStreamer(bool oldFormat = false);
  void setRefCoreStreamer(EDProductGetter const* ep, bool oldFormat = false);
}
#endif
