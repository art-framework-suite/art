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

#include "art/Framework/Core/ModuleBase.h"
#include "art/Framework/Core/ProcessingFrame.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/ModuleTypeDeducer.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ModuleType.h"
#include "cetlib/ProvideFilePathMacro.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/AllowedConfigurationMacro.h"

#include <memory>
#include <type_traits>

namespace art::detail {
  using ModuleMaker_t = ModuleBase*(fhicl::ParameterSet const&,
                                    ProcessingFrame const&);
  using WorkerFromModuleMaker_t = Worker*(std::shared_ptr<ModuleBase>,
                                          ModuleDescription const&,
                                          WorkerParams const&);
  using WorkerMaker_t = Worker*(WorkerParams const&, ModuleDescription const&);
  using ModuleTypeFunc_t = ModuleType();
  using ModuleThreadingTypeFunc_t = ModuleThreadingType();

  template <typename T, typename = void>
  struct config_impl {
    using type = fhicl::ParameterSet;
  };

  template <typename T>
  struct config_impl<T, std::void_t<typename T::Parameters>> {
    using type = typename T::Parameters;
  };

  template <typename T>
  using Config = typename config_impl<T>::type;

  template <typename T, typename Config>
  T*
  make_module(Config const& config, ProcessingFrame const& frame)
  {
    if constexpr (ModuleThreadingTypeDeducer<typename T::ModuleType>::value ==
                  ModuleThreadingType::legacy) {
      return new T{config};
    } else {
      return new T{config, frame};
    }
  }
}

#define DEFINE_ART_MODULE(klass)                                               \
  extern "C" {                                                                 \
  CET_PROVIDE_FILE_PATH()                                                      \
  FHICL_PROVIDE_ALLOWED_CONFIGURATION(klass)                                   \
  art::ModuleBase*                                                             \
  make_module(fhicl::ParameterSet const& pset,                                 \
              art::ProcessingFrame const& frame)                               \
  {                                                                            \
    /* Reference below to avoid copy if Config<klass> is a ParameterSet. */    \
    art::detail::Config<klass> const& config{pset};                            \
    return art::detail::make_module<klass>(config, frame);                     \
  }                                                                            \
  art::Worker*                                                                 \
  make_worker_from_module(std::shared_ptr<art::ModuleBase> mod,                \
                          art::ModuleDescription const& md,                    \
                          art::WorkerParams const& wp)                         \
  {                                                                            \
    return new klass::WorkerType(                                              \
      std::dynamic_pointer_cast<klass::ModuleType>(mod), md, wp);              \
  }                                                                            \
  art::ModuleType                                                              \
  moduleType()                                                                 \
  {                                                                            \
    return art::detail::ModuleTypeDeducer<klass::ModuleType>::value;           \
  }                                                                            \
  art::ModuleThreadingType                                                     \
  moduleThreadingType()                                                        \
  {                                                                            \
    return art::detail::ModuleThreadingTypeDeducer<klass::ModuleType>::value;  \
  }                                                                            \
  }

#endif /* art_Framework_Core_ModuleMacros_h */

// Local Variables:
// mode: c++
// End:
