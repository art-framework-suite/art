// -*- C++ -*-
//
// Package:     PluginManager
// Class  :     DummyFactory
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Fri Apr  6 15:26:46 EDT 2007
//
//

// system include files

// user include files
#include "test/Framework/PluginManager/DummyFactory.h"

namespace testedmplugin {
  DummyBase::~DummyBase() {}
}

EDM_REGISTER_PLUGINFACTORY(testartplugin::DummyFactory,"Test Dummy");
