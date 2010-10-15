#include "art/Framework/Services/Basic/UnixSignalService.h"

#include "art/Framework/Services/Registry/ServiceMaker.h"
#include "art/Utilities/UnixSignalHandlers.h"

#include "fhiclcpp/ParameterSet.h"

#include <cstdlib>
#include <iostream>

using art::service::UnixSignalService;


namespace art {
  namespace service {

    UnixSignalService::UnixSignalService(fhicl::ParameterSet const& pset,
                                         art::ActivityRegistry& registry)
      : enableSigInt_(pset.get<bool>("EnableCtrlC",true))
    {
      art::installCustomHandler(SIGUSR2,art::ep_sigusr2);
      if(enableSigInt_)  art::installCustomHandler(SIGINT ,art::ep_sigusr2);
    }

    UnixSignalService::~UnixSignalService() {}

  } // namespace service
} // namespace art


// ======================================================================


DEFINE_FWK_SERVICE(UnixSignalService);
