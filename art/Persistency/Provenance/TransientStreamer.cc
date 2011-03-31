#include "art/Persistency/Provenance/TransientStreamer.h"

#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/FileIndex.h"
#include "art/Persistency/Provenance/ProcessHistory.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Persistency/Provenance/Transient.h"

namespace art {

  namespace {

    template <typename T>
    void
    SetTransientStreamer() {
      TClass *cl = gROOT->GetClass(TypeID(typeid(T)).className().c_str());
      if (cl->GetStreamer() == 0) {
        cl->AdoptStreamer(new TransientStreamer<T>());
      }
    }

    template <typename T>
    void
    SetTransientStreamer(T const&) {
      TClass *cl = gROOT->GetClass(TypeID(typeid(T)).className().c_str());
      if (cl->GetStreamer() == 0) {
        cl->AdoptStreamer(new TransientStreamer<T>());
      }
    }

  }  // namespace

  void setTransientStreamers()
  {
    SetTransientStreamer<Transient< BranchDescription::Transients> >();
    SetTransientStreamer<Transient< ProductProvenance::Transients> >();
    SetTransientStreamer<Transient< FileIndex        ::Transients> >();
    SetTransientStreamer<Transient< ProcessHistory   ::Transients> >();
    SetTransientStreamer<Transient< ProductRegistry  ::Transients> >();
  }

}  // art
