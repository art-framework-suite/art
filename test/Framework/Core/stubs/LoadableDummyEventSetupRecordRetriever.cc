// -*- C++ -*-
//
// Package:     test
// Class  :     LoadableDummyEventSetupRecordRetriever
//
// Implementation:
//     <Notes on implementation>
//
// Author:      Chris Jones
// Created:     Thu May 26 13:48:03 EDT 2005
//
//

// system include files

// user include files

#include "test/Framework/Core/DummyEventSetupRecordRetriever.h"
#include "art/Framework/Core/SourceFactory.h"


namespace edm {
   class ParameterSet;
}

class LoadableDummyEventSetupRecordRetriever : public edm::DummyEventSetupRecordRetriever
{
public:
  LoadableDummyEventSetupRecordRetriever(const edm::ParameterSet& iPSet) {
   }
};

DEFINE_FWK_EVENTSETUP_SOURCE(LoadableDummyEventSetupRecordRetriever);
