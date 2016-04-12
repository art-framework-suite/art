/*
 *  Modules.cc
 *  CMSSW
 *
 *  Created by Chris Jones on 9/7/05.
 *
 */

#include "FWCore/ServiceRegistry/test/stubs/DependsOnDummyService.h"
#include "FWCore/ServiceRegistry/test/stubs/DummyService.h"
#include "FWCore/ServiceRegistry/test/stubs/DummyServiceE0.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

using namespace testserviceregistry;
using namespace art::serviceregistry;
DEFINE_ART_SERVICE_MAKER(DependsOnDummyService,NoArgsMaker<DependsOnDummyService>)
DEFINE_ART_SERVICE(DummyService)
DEFINE_ART_SERVICE(DummyServiceE0)
DEFINE_ART_SERVICE(DummyServiceA1)
DEFINE_ART_SERVICE(DummyServiceD2)
DEFINE_ART_SERVICE(DummyServiceB3)
DEFINE_ART_SERVICE(DummyServiceC4)
