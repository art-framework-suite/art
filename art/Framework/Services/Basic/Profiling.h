#ifndef FWCore_Services_Profiling_h
#define FWCore_Services_Profiling_h
// -*- C++ -*-
//
// Package:     Services
// Class  :     SimpleProfiling
//
//
// Original Author:  Jim Kowalkowski
//
//

#include "art/ParameterSet/ParameterSet.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"

namespace edm {
  namespace service {
    class SimpleProfiling
    {
    public:
      SimpleProfiling(const ParameterSet&,ActivityRegistry&);
      ~SimpleProfiling();

      void postBeginJob();
      void postEndJob();

    private:
    };
  }
}



#endif
