#include "art/Persistency/Common/CacheStreamers.h"
#include "art/Persistency/Common/ConstPtrCache.h"
#include "art/Persistency/Common/BoolCache.h"
#include "cetlib/exception.h"

// For nullptr:
#include "cpp0x/memory"

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
    TClass *cl = TClass::GetClass(typeid(BoolCache));
    if (cl == nullptr) {
       throw cet::exception("INTERNAL_ERROR")
          << "Could not find dictionary for art::BoolCache.";
    }
    if (cl->GetStreamer() == 0) {
      cl->AdoptStreamer(new BoolCacheStreamer());
    /*} else {
      std::cout <<"ERROR: no art::BoolCache found"<<std::endl;*/
    }

    cl = TClass::GetClass(typeid(ConstPtrCache));
    if (cl == nullptr) {
       throw cet::exception("INTERNAL_ERROR")
          << "Could not find dictionary for art::ConstPtrCache.";
    }
    if (cl->GetStreamer() == 0) {
      cl->AdoptStreamer(new ConstPtrCacheStreamer());
    /*} else {
      std::cout <<"ERROR: no art::ConstPtrCache found"<<std::endl;*/
    }

  }

}
