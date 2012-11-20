// -*- C++ -*-
//
// Package:     Services
// Class  :     Guess
//

// system include files
#include <iostream>
#include <cerrno>
#include <cstdlib>

// user include files
#include "art/Framework/Services/UserInteraction/UserInteraction.h"

#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/Timestamp.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Core/InputSource.h"
#include "fhiclcpp/ParameterSetRegistry.h"

using namespace art;
using namespace std;

namespace ui {

  UserInteraction::UserInteraction(ActivityRegistry & iReg)
  {
    iReg.watchPostBeginJobWorkers(this, &UserInteraction::postBeginJobWorkers);
    //iReg.watchPreModule(this, &Tracer::preModuleEvent);
    //iReg.watchPostModule(this, &UserInteraction::postModuleEvent);
    iReg.watchPreProcessEvent(this, &UserInteraction::preEvent);
    iReg.watchPostProcessEvent(this, &UserInteraction::postEvent);
  }

  void UserInteraction::postBeginJobWorkers(InputSource * is,
      std::vector<art::Worker *> const & workers)
  {
    std::cout << "Post Begin Job Workers" << std::endl;
    std::vector<art::Worker *>::const_iterator ib(workers.begin()), ie(workers.end());
    std::vector<ModuleInfo> mi;
    for (; ib != ie; ++ib) {
      workers_.push_back(*ib);
      const ModuleDescription & md = (*ib)->description();
      //std::cout << md.moduleName() << " " << md.moduleLabel() << "\n";
      mi.push_back(ModuleInfo(md.moduleLabel(),
                              md.moduleName(),
                              fhicl::ParameterSet()));
      fhicl::ParameterSetRegistry::get(md.parameterSetID(),
                                       mi.back().pset);
    }
    moduleList(mi);
    input_ = is;
  }

  void UserInteraction::preEvent(Event const &)
  {
    //std::cout << "event:"<< iID<<" time:"<<iTime.value()<< std::endl;
    pickModule();
  }

  void UserInteraction::postEvent(Event const &)
  {
    NextStep which = nextAction();
    switch (which) {
      case NextEvent:
        break;
      case ReprocessEvent:
        input_->skipEvents(-1);
        break;
      case RewindFile:
        input_->rewind();
        break;
      default:
        break;
    }
  }

  void UserInteraction::callReconfigure(int mod_index,
                                        fhicl::ParameterSet const & pset)
  {
    workers_[mod_index]->reconfigure(pset);
  }


#if 0
  void
  UserInteraction::preModuleEvent(ModuleDescription const & iDescription)
  {
  }
  void
  UserInteraction::postModuleEvent(ModuleDescription const & iDescription)
  {
  }
#endif

}

