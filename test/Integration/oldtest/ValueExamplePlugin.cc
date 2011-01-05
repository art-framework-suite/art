// -*- C++ -*-
//
// Package:     test
// Class  :     ValueExample
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Mon Sep  5 19:52:01 EDT 2005
//
//

// system include files

// user include files
#include "FWCore/Integration/test/ValueExample.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
using namespace art::serviceregistry;
DEFINE_FWK_SERVICE_MAKER(ValueExample,ParameterSetMaker<ValueExample>);
