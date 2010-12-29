#include "art/Persistency/Common/CacheStreamers.h"
#include "art/Persistency/Common/ConstPtrCache.h"
#include "art/Persistency/Common/BoolCache.h"
#include "TROOT.h"
class TBuffer;

namespace art {
  void
  BoolCacheStreamer::operator()(TBuffer &R_b, void *objp) {
    if (R_b.IsReading()) {
      cl_->ReadBuffer(R_b, objp);
      BoolCache* obj = static_cast<BoolCache *>(objp);
      *obj = false;
    } else {
      cl_->WriteBuffer(R_b, objp);
    }
  }

  void
  ConstPtrCacheStreamer::operator()(TBuffer &R_b, void *objp) {
    if (R_b.IsReading()) {
      cl_->ReadBuffer(R_b, objp);
      ConstPtrCache* obj = static_cast<ConstPtrCache *>(objp);
      obj->ptr_=0;
    } else {
      cl_->WriteBuffer(R_b, objp);
    }
  }

  void setCacheStreamers() {
    TClass *cl = gROOT->GetClass("art::BoolCache");
    if (cl->GetStreamer() == 0) {
      cl->AdoptStreamer(new BoolCacheStreamer());
    /*} else {
      std::cout <<"ERROR: no art::BoolCache found"<<std::endl;*/
    }

    cl = gROOT->GetClass("art::ConstPtrCache");
    if (cl->GetStreamer() == 0) {
      cl->AdoptStreamer(new ConstPtrCacheStreamer());
    /*} else {
      std::cout <<"ERROR: no art::ConstPtrCache found"<<std::endl;*/
    }

  }

}
