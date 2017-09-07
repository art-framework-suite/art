#ifndef art_Framework_Core_ModuleMacros_h
#define art_Framework_Core_ModuleMacros_h
// vim: set sw=2 expandtab :

//
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

#include "art/Framework/Core/EventObserverBase.h"
#include "art/Framework/Core/ModuleBase.h"
#include "art/Framework/Core/ModuleType.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/ModuleTypeDeducer.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "cetlib/ProvideFilePathMacro.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/AllowedConfigurationMacro.h"

#include <memory>
#include <ostream>
#include <string>

namespace art {
namespace detail {

using ModuleMaker_t = ModuleBase*(art::ModuleDescription const&, WorkerParams const&);
using WorkerFromModuleMaker_t = Worker*(ModuleBase*, ModuleDescription const&, WorkerParams const&);
using WorkerMaker_t = Worker*(WorkerParams const&, ModuleDescription const&);
using ModuleTypeFunc_t = ModuleType();
using ModuleThreadingTypeFunc_t = ModuleThreadingType();

} // namespace detail
} // namespace art

#define DEFINE_ART_MODULE_NEW(klass) \
  extern "C" { \
    CET_PROVIDE_FILE_PATH() \
    FHICL_PROVIDE_ALLOWED_CONFIGURATION(klass) \
    art::ModuleBase* \
    make_module(art::ModuleDescription const& md, art::WorkerParams const& wp) \
    { \
      art::ModuleBase* mod = new klass(wp.pset_); \
      mod->setModuleDescription(md); \
      mod->setStreamIndex(wp.streamIndex_); \
      return mod; \
    } \
    art::Worker* \
    make_worker_from_module(art::ModuleBase* mod, art::ModuleDescription const& md, art::WorkerParams const& wp) \
    { \
      return new klass::WorkerType(dynamic_cast<klass::ModuleType*>(mod), md, wp); \
    } \
    art::ModuleType \
    moduleType() \
    { \
      return art::detail::ModuleTypeDeducer<klass::ModuleType>::value; \
    } \
  }

#define DEFINE_ART_LEGACY_MODULE(klass) \
  DEFINE_ART_MODULE_NEW(klass) \
  extern "C" { \
    art::ModuleThreadingType \
    moduleThreadingType() \
    { \
      return art::ModuleThreadingType::LEGACY; \
    } \
  }

#define DEFINE_ART_ONE_MODULE(klass) \
  DEFINE_ART_MODULE_NEW(klass) \
  extern "C" { \
    art::ModuleThreadingType \
    moduleThreadingType() \
    { \
      return art::ModuleThreadingType::ONE; \
    } \
  }

#define DEFINE_ART_STREAM_MODULE(klass) \
  DEFINE_ART_MODULE_NEW(klass) \
  extern "C" { \
    art::ModuleThreadingType \
    moduleThreadingType() \
    { \
      return art::ModuleThreadingType::STREAM; \
    } \
  }

#define DEFINE_ART_GLOBAL_MODULE(klass) \
  DEFINE_ART_MODULE_NEW(klass) \
  extern "C" { \
    art::ModuleThreadingType \
    moduleThreadingType() \
    { \
      return art::ModuleThreadingType::GLOBAL; \
    } \
  }

#define DEFINE_ART_MODULE(klass) \
  DEFINE_ART_LEGACY_MODULE(klass)

#endif /* art_Framework_Core_ModuleMacros_h */

// Local Variables:
// mode: c++
// End:
