#ifndef FWCore_Services_SimpleInteraction_h
#define FWCore_Services_SimpleInteraction_h

// -*- C++ -*-
//
// Package:     Services
// Class  :     UserInteraction
// 
/*

 Description: Allows for reconfiguration of modules at 
   start of event loop, and reprocessing, rewind, or continuation at the end.

*/
//
// Original Author:  Jim Kowalkowski and Marc Paterno
//         Created:  Wed Sep 27 2010
//

#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Optional/UserInteraction.h"
#include <vector>

namespace ui {

  class SimpleInteraction : public UserInteraction
  {
  public:
    SimpleInteraction(const fhicl::ParameterSet&,art::ActivityRegistry&);

    void moduleList(std::vector<ModuleInfo> const&);
    void pickModule();
    UserInteraction::NextStep nextAction();

  private:
    std::vector<ModuleInfo> infos_;
  };
}
   
#endif
