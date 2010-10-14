#include "art/Framework/Services/Basic/UnixSignalService.h"

#include "art/Framework/Services/Registry/ServiceMaker.h"
#include "art/Utilities/UnixSignalHandlers.h"

#include "fhiclcpp/ParameterSet.h"

#include <cstdlib>
#include <iostream>

using edm::service::UnixSignalService;


namespace edm {
  namespace service {

    UnixSignalService::UnixSignalService(fhicl::ParameterSet const& pset,
                                         edm::ActivityRegistry& registry)
      : enableSigInt_(pset.get<bool>("EnableCtrlC",true))
    {
      edm::installCustomHandler(SIGUSR2,edm::ep_sigusr2);
      if(enableSigInt_)  edm::installCustomHandler(SIGINT ,edm::ep_sigusr2);
    }

    UnixSignalService::~UnixSignalService() {}

  } // namespace service
} // namespace edm


// ======================================================================


DEFINE_FWK_SERVICE(UnixSignalService);
