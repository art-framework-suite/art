#ifndef Framework_MakerMacros_h
#define Framework_MakerMacros_h

#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Framework/Core/WorkerParams.h"

/*
  Note: Libraries that include these symbol definitions cannot be
  linked into a main program as other libraries are.  This is because
  the "one definition" rule will be violated.
 */

/*
  DEFINE_ART_MODULE_TEMP is a short-term expedient, to get an early
  version of art working. It will be removed as soon as feasible.
 */
#define DEFINE_ART_MODULE_TEMP(klass) \
extern "C" \
Worker* make_temp(WorkerParams const& wp, ModuleDescription const& md) \
{ return new WorkerT<klass::ModuleType>(new klass(*(wp.pset_)), md, wp)); }

/*
  DEFINE_ART_MODULE produces the function that is used to create a
  module instance.
*/

#define DEFINE_ART_MODULE(klass) \
extern "C" \
klass::ModuleType* make(fhicl::ParameterSet const& ps) \
{ return new klass(ps); } \
 DEFINE_ART_MODULE_TEMP(klass)

#endif  // Framework_MakerMacros_h
