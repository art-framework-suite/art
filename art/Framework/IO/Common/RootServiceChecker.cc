#include "art/Framework/IO/Common/RootServiceChecker.h"
#include "art/Framework/Services/Registry/Service.h"
#include "art/Utilities/EDMException.h"
#include "art/Utilities/RootHandlers.h"

namespace art {
  RootServiceChecker::RootServiceChecker() {
    Service<RootHandlers> rootSvc;
    if (!rootSvc.isAvailable()) {
      throw art::Exception(errors::Configuration) <<
	    "The 'InitRootHandlers' service was not specified.\n" <<
	    "This service must be used if PoolSource or PoolOutputModule is used.\n";
    }
  }
}

