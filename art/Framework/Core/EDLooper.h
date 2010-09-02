#ifndef FWCore_Framework_EDLooper_h
#define FWCore_Framework_EDLooper_h

// -*- C++ -*-
//
// Package:     Framework
// Module:      EDLooper
//
/**\class EDLooper EDLooper.h package/EDLooper.h

 Description: Base class for all looping components
*/
//
// Author:      Valentin Kuznetsov
// Created:     Wed Jul  5 11:42:17 EDT 2006
//
//

#include "art/Framework/Core/Frameworkfwd.h"

#include <set>

namespace edm {
  class ActionTable;

  class EDLooper
  {
    public:

      enum Status {kContinue, kStop};

      EDLooper();
      virtual ~EDLooper();

      void doStartingNewLoop();
      Status doDuringLoop(edm::EventPrincipal& eventPrincipal);
      Status doEndOfLoop();
      void prepareForNextLoop();

      virtual void beginOfJob();
      virtual void startingNewLoop(unsigned int ) = 0;
      virtual Status duringLoop(const edm::Event&) = 0;
      virtual Status endOfLoop(unsigned int iCounter) = 0;
      virtual void endOfJob();

      void setActionTable(ActionTable* actionTable) { act_table_ = actionTable; }

      virtual std::set<> modifyingRecords() const;

    private:

      EDLooper( const EDLooper& ); // stop default
      const EDLooper& operator=( const EDLooper& ); // stop default

      unsigned int iCounter_;
      ActionTable* act_table_;
  };
}

#endif  // FWCore_Framework_EDLooper_h
