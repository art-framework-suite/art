#include "art/Persistency/Common/priv/setPtrVectorBaseStreamer.h"

#include "art/Persistency/Common/PtrVectorBase.h"
#include "art/Utilities/TypeID.h"

#include "TClass.h"
#include "TROOT.h"

void
art::priv::PtrVectorBaseStreamer::operator()(TBuffer &R_b, void *objp) {
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
art::priv::setPtrVectorBaseStreamer() {
  TClass *cl = gROOT->GetClass(TypeID(typeid(PtrVectorBase)).className().c_str());
  if (cl->GetStreamer() == 0) {
    cl->AdoptStreamer(new PtrVectorBaseStreamer());
  }
}
