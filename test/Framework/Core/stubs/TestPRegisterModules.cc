// -*- C++ -*-
//
// Package:     test
// Class  :     TestPRegisterModules
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Sat Sep 24 10:58:40 CEST 2005
//
//

// system include files

// user include files
#include "art/Framework/Core/ModuleMacros.h"
#include "FWCore/Framework/test/stubs/TestPRegisterModule1.h"
#include "FWCore/Framework/test/stubs/TestPRegisterModule2.h"

DEFINE_ART_MODULE(TestPRegisterModule1);
DEFINE_ART_MODULE(TestPRegisterModule2);
