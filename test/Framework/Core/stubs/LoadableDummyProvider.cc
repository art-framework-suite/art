// -*- C++ -*-
//
// Package:     test
// Class  :     LoadableDummyProvider
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
#include "test/Framework/Core/DummyProxyProvider.h"

#include "art/Framework/Core/ModuleFactory.h"

namespace art {
   class ParameterSet;
}
class LoadableDummyProvider : public art::eventsetup::test::DummyProxyProvider {
public:
   LoadableDummyProvider(const art::ParameterSet& iPSet)
   :DummyProxyProvider( art::eventsetup::test::DummyData(iPSet.getUntrackedParameter<int>("value",1))) {}
};

DEFINE_FWK_EVENTSETUP_MODULE(LoadableDummyProvider);
