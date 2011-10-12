#include "art/Framework/IO/Root/RefCoreStreamer.h"

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Persistency/Common/RefCore.h"
#include "art/Utilities/Exception.h"

#include "TROOT.h"


#include "messagefacility/MessageLogger/MessageLogger.h"
#include <iomanip>
#include <iostream>

namespace art {
  void
  RefCoreStreamer::operator()(TBuffer & R_b, void * objp)
  {
    static TClassRef cl("art::RefCore");
    if (R_b.IsReading()) {
      cl->ReadBuffer(R_b, objp);
      RefCore * obj = static_cast<RefCore *>(objp);
      if (groupFinder_) {
        obj->setProductGetter(groupFinder_->getGroup(obj->id()).result().get());
      }
      else {
        obj->setProductGetter(0);
      }
      obj->setProductPtr(0);
    }
    else {
      cl->WriteBuffer(R_b, objp);
    }
  }

  void configureRefCoreStreamer(cet::exempt_ptr<EventPrincipal const> groupFinder)
  {
    static TClassRef cl("art::RefCore");
    RefCoreStreamer * st = static_cast<RefCoreStreamer *>(cl->GetStreamer());
    if (st == 0) {
      cl->AdoptStreamer(new RefCoreStreamer(groupFinder));
    }
    else {
      st->setGroupFinder(groupFinder);
    }
  }
}
