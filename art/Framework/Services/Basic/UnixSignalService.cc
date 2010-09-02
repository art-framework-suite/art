#include <cstdlib>
#include <iostream>

#include "art/Framework/Services/Basic/UnixSignalService.h"
#include "art/ParameterSet/ParameterSet.h"
#include "art/Utilities/UnixSignalHandlers.h"


namespace edm {
  namespace service {

    UnixSignalService::UnixSignalService(edm::ParameterSet const& pset,
                                         edm::ActivityRegistry& registry)
      : enableSigInt_(pset.getUntrackedParameter<bool>("EnableCtrlC",true))
    {
      edm::installCustomHandler(SIGUSR2,edm::ep_sigusr2);
      if(enableSigInt_)  edm::installCustomHandler(SIGINT ,edm::ep_sigusr2);
    }

    UnixSignalService::~UnixSignalService() {}

  } // namespace service
} // namespace edm
