#ifndef Framework_MakerMacros_h
#define Framework_MakerMacros_h

#include "art/Framework/Core/Factory.h"
#include "art/Framework/Core/WorkerMaker.h"
// The following includes are temporary until a better
// solution can be found.  Placing these includes here
// leads to more physical coupling than is probably necessary.
// Another solution is to build a typeid lookup table in the
// implementation file (one every for each XXXWorker) and
// then include all the relevent worker headers in the
// implementation file only.
#include "art/Framework/Core/OutputWorker.h"
#include "art/Framework/Core/WorkerT.h"


#define DEFINE_FWK_MODULE(type) \
  //DEFINE_EDM_PLUGIN (edm::MakerPluginFactory,edm::WorkerMaker<type>,#type); DEFINE_FWK_PSET_DESC_FILLER(type)

#define DEFINE_ANOTHER_FWK_MODULE(type) \
  //DEFINE_EDM_PLUGIN (edm::MakerPluginFactory,edm::WorkerMaker<type>,#type); DEFINE_FWK_PSET_DESC_FILLER(type)

// for backward compatibility
#include "art/Framework/PluginManager/ModuleDef.h"

#endif  // Framework_MakerMacros_h
