/*----------------------------------------------------------------------
----------------------------------------------------------------------*/

#include "art/Framework/Core/NoDelayedReader.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Framework/Core/LuminosityBlockPrincipal.h"
#include "art/Utilities/Exception.h"

namespace edm {
  NoDelayedReader::~NoDelayedReader() {}

  std::auto_ptr<EDProduct>
  NoDelayedReader::getProduct_(BranchKey const& k, EDProductGetter const* ep) const {
    EventPrincipal const* epr = dynamic_cast<EventPrincipal const*>(ep);
    if (epr) {
      throw edm::Exception(errors::LogicError,"NoDelayedReader")
        << "getProduct() called for branchkey: " << k << " EventID: " << epr->id() << "\n";
    }
    RunPrincipal const* rpr = dynamic_cast<RunPrincipal const*>(ep);
    if (rpr) {
      throw edm::Exception(errors::LogicError,"NoDelayedReader")
        << "getProduct() called for branchkey: " << k << " RunID: " << epr->id() << "\n";
    }
    LuminosityBlockPrincipal const* lpr = dynamic_cast<LuminosityBlockPrincipal const*>(ep);
    if (lpr) {
      throw edm::Exception(errors::LogicError,"NoDelayedReader")
        << "getProduct() called for branchkey: " << k << " LuminosityBlockNumber_t: " << lpr->id() << "\n";
    }
    throw edm::Exception(errors::LogicError,"NoDelayedReader")
      << "getProduct() called for branchkey: " << k << "\n";
  }
}
