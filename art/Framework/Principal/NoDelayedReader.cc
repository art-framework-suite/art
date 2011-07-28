/*----------------------------------------------------------------------
----------------------------------------------------------------------*/

#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "cetlib/exception.h"

namespace art {
  NoDelayedReader::~NoDelayedReader() {}

  std::auto_ptr<EDProduct>
  NoDelayedReader::getProduct_(BranchKey const& k, EDProductGetter const* ep) const {
    EventPrincipal const* epr = dynamic_cast<EventPrincipal const*>(ep);
    if (epr) {
      throw art::Exception(errors::LogicError,"NoDelayedReader")
        << "getProduct() called for branchkey: " << k << " EventID: " << epr->id() << "\n";
    }
    RunPrincipal const* rpr = dynamic_cast<RunPrincipal const*>(ep);
    if (rpr) {
      throw art::Exception(errors::LogicError,"NoDelayedReader")
        << "getProduct() called for branchkey: " << k << " RunID: " << epr->id() << "\n";
    }
    SubRunPrincipal const* lpr = dynamic_cast<SubRunPrincipal const*>(ep);
    if (lpr) {
      throw art::Exception(errors::LogicError,"NoDelayedReader")
        << "getProduct() called for branchkey: " << k << " SubRunNumber_t: " << lpr->id() << "\n";
    }
    throw art::Exception(errors::LogicError,"NoDelayedReader")
      << "getProduct() called for branchkey: " << k << "\n";
  }
}
