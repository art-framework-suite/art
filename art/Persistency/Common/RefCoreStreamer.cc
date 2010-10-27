#include "art/Persistency/Common/RefCoreStreamer.h"
#include "art/Persistency/Common/RefCore.h"
#include "art/Utilities/Exception.h"
#include "TROOT.h"
#include <ostream>
#include <cassert>

namespace art {
  void
  ProductIDStreamer::operator()(TBuffer &R__b, void *objp) {
    if (R__b.IsReading()) {
      UInt_t i0, i1;
      R__b.ReadVersion(&i0, &i1, cl_);
      unsigned int id;
      R__b >> id;
      ProductID pid;
      pid.oldID() = id;
      ProductID* obj = static_cast<ProductID *>(objp);
      *obj = (prodGetter_ ? prodGetter_->oldToNewProductID(pid) : pid);
    } else {
      assert("ProductID streamer is obsolete" == 0);
    }
  }

  void
  RefCoreStreamer::operator()(TBuffer &R__b, void *objp) {
    if (R__b.IsReading()) {
      cl_->ReadBuffer(R__b, objp);
      RefCore* obj = static_cast<RefCore *>(objp);
      obj->setProductGetter(prodGetter_);
      obj->setProductPtr(0);
    } else {
      assert("RefCore streamer is obsolete" == 0);
    }
  }

  void
  RefCoreTransientStreamer::operator()(TBuffer &R__b, void *objp) {
    typedef RefCore::RefCoreTransients RefCoreTransients;
    if (R__b.IsReading()) {
      cl_->ReadBuffer(R__b, objp);
      RefCoreTransients* obj = static_cast<RefCoreTransients *>(objp);
      obj->setProductGetter(prodGetter_);
      obj->setProductPtr(0);
    } else {
      RefCoreTransients* obj = static_cast<RefCoreTransients *>(objp);
      if (obj->isTransient()) {
        throw Exception(errors::InvalidReference,"Inconsistency")
          << "RefCoreStreamer: transient Ref or Ptr cannot be made persistent.";
      }
      cl_->WriteBuffer(R__b, objp);
    }
  }

  void setRefCoreStreamer(bool oldFormat) {
    {
      TClass *cl = gROOT->GetClass("art::RefCore::RefCoreTransients");
      RefCoreTransientStreamer *st = static_cast<RefCoreTransientStreamer *>(cl->GetStreamer());
      if (st == 0) {
        cl->AdoptStreamer(new RefCoreTransientStreamer(0));
      } else {
        st->setProductGetter(0);
      }
    }
  }

  void setRefCoreStreamer(EDProductGetter const* ep, bool oldFormat) {
    if (ep != 0) {
        TClass *cl = gROOT->GetClass("art::RefCore::RefCoreTransients");
        RefCoreTransientStreamer *st = static_cast<RefCoreTransientStreamer *>(cl->GetStreamer());
        if (st == 0) {
          cl->AdoptStreamer(new RefCoreTransientStreamer(ep));
        } else {
          st->setProductGetter(ep);
        }
    }
  }
}
