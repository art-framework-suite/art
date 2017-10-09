#ifndef art_Framework_Core_ModuleMacros_h
#define art_Framework_Core_ModuleMacros_h

////////////////////////////////////////////////////////////////////////
// ModuleMacros
//
// Defines the macro DEFINE_ART_MODULE(<module_classname>) to be used in
// XXX_module.cc to declare art modules (analyzers, filters, producers
// and outputs).
//
// Note: Libraries that include these symbol definitions cannot be
// linked into a main program as other libraries are.  This is because
// the "one definition" rule would be violated.
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/ModuleType.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/ModuleTypeDeducer.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "cetlib/ProvideFilePathMacro.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/AllowedConfigurationMacro.h"

#include "art/Framework/Core/EventObserverBase.h"

#include <memory>
#include <ostream>
#include <string>

// Function aliases (used in art).
namespace art {
  namespace detail {
    using WorkerMaker_t = Worker*(WorkerParams const&,
                                  ModuleDescription const&);
    using ModuleTypeFunc_t = ModuleType();
  }
}

// Produce the injected functions
#define DEFINE_ART_MODULE(klass)                                               \
  extern "C" {                                                                 \
  CET_PROVIDE_FILE_PATH()                                                      \
  FHICL_PROVIDE_ALLOWED_CONFIGURATION(klass)                                   \
  art::Worker*                                                                 \
  make_worker(art::WorkerParams const& wp, art::ModuleDescription const& md)   \
  {                                                                            \
    return new klass::WorkerType(                                              \
      std::unique_ptr<klass::ModuleType>(new klass(wp.pset_)), md, wp);        \
  }                                                                            \
  art::ModuleType                                                              \
  moduleType()                                                                 \
  {                                                                            \
    return art::detail::ModuleTypeDeducer<klass::ModuleType>::value;           \
  }                                                                            \
  }

#endif /* art_Framework_Core_ModuleMacros_h */

// Local Variables:
// mode: c++
// End:
