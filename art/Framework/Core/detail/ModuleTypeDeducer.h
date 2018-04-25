#ifndef art_Framework_Core_detail_ModuleTypeDeducer_h
#define art_Framework_Core_detail_ModuleTypeDeducer_h
// vim: set sw=2 expandtab :

//
//  Used by ModuleMacros.h to ascertain the correct
//  ModuleType for a module.
//

#include "art/Framework/Core/ModuleType.h"

namespace art {
  class EDAnalyzer;
  class EDProducer;
  class EDFilter;
  class OutputModule;

  class SharedAnalyzer;
  class SharedProducer;
  class SharedFilter;
  class SharedOutputModule;

  class ReplicatedAnalyzer;
  class ReplicatedProducer;
  class ReplicatedFilter;
  class ReplicatedOutputModule;

  namespace detail {

    // Only specializations allowed so that errors can be caught at
    // compile-time and not run-time.
    template <typename T>
    struct ModuleTypeDeducer;

    // Legacy modules
    template <>
    struct ModuleTypeDeducer<EDAnalyzer> {
      static constexpr ModuleType value = ModuleType::analyzer;
    };

    template <>
    struct ModuleTypeDeducer<EDFilter> {
      static constexpr ModuleType value = ModuleType::filter;
    };

    template <>
    struct ModuleTypeDeducer<OutputModule> {
      static constexpr ModuleType value = ModuleType::output_module;
    };

    template <>
    struct ModuleTypeDeducer<EDProducer> {
      static constexpr ModuleType value = ModuleType::producer;
    };

    template <>
    struct ModuleTypeDeducer<SharedAnalyzer> {
      static constexpr ModuleType value = ModuleType::analyzer;
    };

    template <>
    struct ModuleTypeDeducer<SharedFilter> {
      static constexpr ModuleType value = ModuleType::filter;
    };

    template <>
    struct ModuleTypeDeducer<SharedOutputModule> {
      static constexpr ModuleType value = ModuleType::output_module;
    };

    template <>
    struct ModuleTypeDeducer<SharedProducer> {
      static constexpr ModuleType value = ModuleType::producer;
    };

    template <>
    struct ModuleTypeDeducer<ReplicatedAnalyzer> {
      static constexpr ModuleType value = ModuleType::analyzer;
    };

    template <>
    struct ModuleTypeDeducer<ReplicatedFilter> {
      static constexpr ModuleType value = ModuleType::filter;
    };

    template <>
    struct ModuleTypeDeducer<ReplicatedOutputModule> {
      static constexpr ModuleType value = ModuleType::output_module;
    };

    template <>
    struct ModuleTypeDeducer<ReplicatedProducer> {
      static constexpr ModuleType value = ModuleType::producer;
    };

  } // namespace detail
} // namespace art

#endif /* art_Framework_Core_detail_ModuleTypeDeducer_h */

// Local Variables:
// mode: c++
// End:
