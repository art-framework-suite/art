#include "art/Persistency/Common/detail/setPtrVectorBaseStreamer.h"

#include "art/Persistency/Common/PtrVectorBase.h"
#include "art/Utilities/TypeID.h"

#include "TClass.h"

// FIXME: This should go away as soon as ROOT makes this function
// public. In the meantime, we have to verify that this signature does
// not change in new versions of ROOT.
namespace ROOT {
  namespace Cintex {
    std::string CintName(const std::string&);
  }
}

void
art::detail::PtrVectorBaseStreamer::operator()(TBuffer &R_b, void *objp) {
  static TClassRef cl("art::PtrVectorBase");
  PtrVectorBase* obj = reinterpret_cast<PtrVectorBase *>(objp);
  if (R_b.IsReading()) {
    obj->zeroTransients(); // Clear transient rep.
    cl->ReadBuffer(R_b, objp);
  } else {
    obj->fill_offsets(obj->indices_); // Fill persistent rep.
    cl->WriteBuffer(R_b, objp);
    PtrVectorBase::indices_t tmp;
    tmp.swap(obj->indices_); // Clear, no longer needed.
  }
}

void
art::detail::setPtrVectorBaseStreamer() {
  TClass *cl = TClass::GetClass(typeid(PtrVectorBase));
  if (cl->GetStreamer() == 0) {
    cl->AdoptStreamer(new PtrVectorBaseStreamer);
  }
}
