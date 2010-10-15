// -*- C++ -*-
//
// Package:     test
// Class  :     DummyFinder
//
// Implementation:
//     <Notes on implementation>
//
// Author:      Chris Jones
// Created:     Thu May 26 10:52:25 EDT 2005
//
//

// system include files

// user include files
#include "test/Framework/Core/DummyFinder.h"
#include "art/Framework/Core/SourceFactory.h"

namespace art {
   class ParameterSet;
}
class LoadableDummyFinder : public DummyFinder {
public:
   LoadableDummyFinder(const art::ParameterSet&) {}
};

DEFINE_FWK_EVENTSETUP_SOURCE(LoadableDummyFinder);
