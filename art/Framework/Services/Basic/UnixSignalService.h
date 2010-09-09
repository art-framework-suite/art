#ifndef FWCore_Services_UnixSignalService_h
#define FWCore_Services_UnixSignalService_h


/*----------------------------------------------------------------------

UnixSignalService: At present, this defines a SIGUSR2 handler and
sets the shutdown flag when that signal has been raised.

This service is instantiated at job startup.

----------------------------------------------------------------------*/


#include "fhicl/ParameterSet.h"


namespace edm {
  class ActivityRegistry;
  class Event;

  namespace service {

    class UnixSignalService
    {
    private:
      bool enableSigInt_;

    public:
      UnixSignalService( fhicl::ParameterSet const& ps
                       , edm::ActivityRegistry& ac );
      ~UnixSignalService();

    }; // UnixSignalService

  }  // namespace service
}  // namespace edm

#endif  // FWCore_Services_UnixSignalService_h
