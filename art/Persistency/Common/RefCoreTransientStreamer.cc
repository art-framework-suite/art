#include "art/Persistency/Common/RefCoreTransientStreamer.h"
#include "art/Persistency/Common/RefCore.h"
#include "art/Utilities/Exception.h"
#include "TROOT.h"

namespace art {
  void
  RefCoreTransientStreamer::operator()(TBuffer &R_b, void *objp) {
    static TClassRef cl("art::RefCore::RefCoreTransients");
    typedef RefCore::RefCoreTransients RefCoreTransients;
    if (R_b.IsReading()) {
      cl->ReadBuffer(R_b, objp);
      RefCoreTransients* obj = static_cast<RefCoreTransients *>(objp);
      obj->setProductGetter(prodGetter_);
      obj->setProductPtr(0);
    } else {
      RefCoreTransients* obj = static_cast<RefCoreTransients *>(objp);
      if (obj->isTransient()) {
        throw Exception(errors::InvalidReference,"Inconsistency")
          << "RefCoreTransientStreamer: transient Ref or Ptr cannot be made persistent.";
      }
      cl->WriteBuffer(R_b, objp);
    }
  }

  void configureRefCoreTransientStreamer(EDProductGetter const* ep) {
    static TClassRef cl("art::RefCore::RefCoreTransients");
    RefCoreTransientStreamer *st = static_cast<RefCoreTransientStreamer *>(cl->GetStreamer());
    if (st == 0) {
      cl->AdoptStreamer(new RefCoreTransientStreamer(ep));
    } else {
      st->setProductGetter(ep);
    }
  }
}
