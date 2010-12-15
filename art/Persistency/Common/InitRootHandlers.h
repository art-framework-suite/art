#ifndef FWCore_Services_InitRootHandlers_h
#define FWCore_Services_InitRootHandlers_h


#include "art/Utilities/RootHandlers.h"

#include "fhiclcpp/ParameterSet.h"


namespace art {
  class ActivityRegistry;

  namespace service {

    class InitRootHandlers : public RootHandlers
    {
    public:
      InitRootHandlers( fhicl::ParameterSet const & pset
                      , art::ActivityRegistry & activity );
      virtual ~InitRootHandlers ();

    private:
      virtual void disableErrorHandler_();
      virtual void enableErrorHandler_();
      bool unloadSigHandler_;
      bool resetErrHandler_;
      bool autoLibraryLoader_;
    };

  }  // namespace service
}  // namespace art

#endif  // InitRootHandlers_H
