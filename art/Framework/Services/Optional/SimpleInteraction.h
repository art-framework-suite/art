#ifndef art_Framework_Services_Optional_SimpleInteraction_h
#define art_Framework_Services_Optional_SimpleInteraction_h

// -*- C++ -*-
//
// Package:     Services
// Class  :     Interfaces/
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
#include "art/Framework/Services/Interfaces/UserInteraction.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include <vector>

namespace ui {

  class SimpleInteraction : public UserInteraction {
  public:
    SimpleInteraction(const fhicl::ParameterSet &, art::ActivityRegistry &);

    void moduleList(std::vector<ModuleInfo> const &);
    void pickModule();
    UserInteraction::NextStep nextAction();

  private:
    std::vector<ModuleInfo> infos_;
  };
}

DECLARE_ART_SERVICE(ui::SimpleInteraction, LEGACY)
#endif /* art_Framework_Services_Optional_SimpleInteraction_h */

// Local Variables:
// mode: c++
// End:
