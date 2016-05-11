// -*- C++ -*-
//
// Package:     Services
// Class  :     Guess
//

// system include files
#include <cerrno>
#include <cstdlib>

// user include files
#include "art/Framework/Services/UserInteraction/UserInteraction.h"

#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/Timestamp.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Core/InputSource.h"
#include "fhiclcpp/ParameterSetRegistry.h"

using namespace art;
using namespace std;

namespace ui {

  UserInteraction::UserInteraction(ActivityRegistry & iReg)
  {
    iReg.sPostBeginJobWorkers.watch(this, &UserInteraction::postBeginJobWorkers);
    iReg.sPreProcessEvent.watch(this, &UserInteraction::preEvent);
    iReg.sPostProcessEvent.watch(this, &UserInteraction::postEvent);
  }

  void UserInteraction::postBeginJobWorkers(InputSource * is,
                                            std::vector<art::Worker *> const & workers)
  {
    std::vector<ModuleInfo> mi;
    for (auto const w : workers) {
      workers_.push_back(w);
      const ModuleDescription & md = w->description();
      mi.emplace_back(md.moduleLabel(),
                      md.moduleName(),
                      !w->modifiesEvent(),
                      fhicl::ParameterSet());
      fhicl::ParameterSetRegistry::get(md.parameterSetID(),
                                       mi.back().pset);
    }
    moduleList(mi);
    input_ = is;
  }

  void UserInteraction::preEvent(Event const &)
  {
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
