#ifndef FWCore_Services_Profiling_h
#define FWCore_Services_Profiling_h

//
// Package:     Services
// Class  :     SimpleProfiling
//


#include "art/Framework/Services/Registry/ActivityRegistry.h"

#include "fhiclcpp/ParameterSet.h"


namespace edm {
  namespace service {

    class SimpleProfiling
    {
    public:
      SimpleProfiling(const fhicl::ParameterSet&,ActivityRegistry&);
      ~SimpleProfiling();

      void postBeginJob();
      void postEndJob();

    private:
    };

  }  // namespace service
}  //namespace edm

#endif  // FWCore_Services_Profiling_h
