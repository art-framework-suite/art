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

#include "art/Framework/Principal/WorkerParams.h"
#include "art/Framework/Core/ModuleType.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/ModuleTypeDeducer.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"

#include "art/Framework/Core/EventObserver.h"

#include <type_traits>

// Function typedefs (used in art).
namespace art {
  namespace detail {
    typedef art::Worker* (WorkerMaker_t) (art::WorkerParams const&,
                                          art::ModuleDescription const&);
    typedef art::ModuleType (ModuleTypeFunc_t) ();
  }
}

// Produce the injected functions
#define DEFINE_ART_MODULE(klass)                                        \
  extern "C" {                                                          \
    art::Worker*                                                        \
    make_worker(art::WorkerParams const& wp,                            \
                art::ModuleDescription const& md)                       \
    {                                                                   \
      return new                                                        \
        klass::WorkerType(std::unique_ptr<klass::ModuleType>            \
                          (new klass(wp.pset_)), md, wp);               \
    }                                                                   \
    art::ModuleType                                                     \
    moduleType()                                                        \
    {                                                                   \
      return art::detail::ModuleTypeDeducer<klass::ModuleType>::value;  \
    }                                                                   \
  }

#endif /* art_Framework_Core_ModuleMacros_h */

// Local Variables:
// mode: c++
// End:
