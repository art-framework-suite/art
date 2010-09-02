//
// Package:     <package>
// Module:      EDLooper
//
// Author:      Valentin Kuznetsov
// Created:     Wed Jul  5 11:44:26 EDT 2006
//

#include "art/Framework/Core/Actions.h"
#include "art/Framework/Core/EDLooper.h"
#include "art/Framework/Core/Event.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Utilities/Algorithms.h"
#include "art/Utilities/Exception.h"

#include "boost/bind.hpp"


namespace edm {

  EDLooper::EDLooper() : iCounter_(0) { }
  EDLooper::~EDLooper() { }

  void
  EDLooper::doStartingNewLoop() { startingNewLoop(iCounter_); }

  EDLooper::Status
  EDLooper::doDuringLoop(edm::EventPrincipal& eventPrincipal) {
    edm::ModuleDescription modDesc;
    modDesc.moduleName_="EDLooper";
    modDesc.moduleLabel_="";
    Event event(eventPrincipal, modDesc);

    Status status = kContinue;
    try {
      status = duringLoop(event);
    }
    catch(cms::Exception& e) {
      actions::ActionCodes action = (act_table_->find(e.rootCause()));
      if (action != actions::Rethrow) {
        LogWarning(e.category())
          << "An exception occurred in the looper, continuing with the next event\n"
          << e.what();
      }
      else {
        throw;
      }
    }
    return status;
  }

  EDLooper::Status EDLooper::doEndOfLoop() { return endOfLoop(iCounter_); }

  void EDLooper::prepareForNextLoop() { ++iCounter_; }

  void EDLooper::beginOfJob() { }

  void EDLooper::endOfJob() { }

}
