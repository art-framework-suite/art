#ifndef art_Framework_Core_ModuleMacros_h
#define art_Framework_Core_ModuleMacros_h

// ======================================================================
//
// ModuleMacros
//
// Note: Libraries that include these symbol definitions cannot be linked
// into a main program as other libraries are.  This is because the "one
// definition" rule would be violated.
//
// ======================================================================

#include "art/Framework/Principal/WorkerParams.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"

// DEFINE_ART_MODULE_TEMP is a short-term expedient, to get an early
// version of art working. It will be removed as soon as feasible.
#define DEFINE_ART_MODULE_TEMP(klass) \
  extern "C" \
  art::Worker* make_temp(art::WorkerParams const& wp, art::ModuleDescription const& md) \
  { return new klass::WorkerType(std::auto_ptr<klass::ModuleType>(new klass(*(wp.pset_))), md, wp); }

// produce the function that is used to create a module instance.
#define DEFINE_ART_MODULE(klass) \
  extern "C" \
  std::auto_ptr<klass::ModuleType> make(fhicl::ParameterSet const& ps) \
  { return std::auto_ptr<klass::ModuleType>(new klass(ps)); } \
  DEFINE_ART_MODULE_TEMP(klass)

// ======================================================================

#endif /* art_Framework_Core_ModuleMacros_h */

// Local Variables:
// mode: c++
// End:
