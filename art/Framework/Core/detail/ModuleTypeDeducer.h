#ifndef art_Framework_Core_detail_ModuleTypeDeducer_h
#define art_Framework_Core_detail_ModuleTypeDeducer_h
// vim: set sw=2 expandtab :

//
//  Used by ModuleMacros.h to ascertain the correct
//  ModuleType for a module.
//

#include "art/Persistency/Provenance/ModuleType.h"

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
      static constexpr auto value{ModuleType::analyzer};
    };

    template <>
    struct ModuleTypeDeducer<EDFilter> {
      static constexpr auto value{ModuleType::filter};
    };

    template <>
    struct ModuleTypeDeducer<OutputModule> {
      static constexpr auto value{ModuleType::output_module};
    };

    template <>
    struct ModuleTypeDeducer<EDProducer> {
      static constexpr auto value{ModuleType::producer};
    };

    // Shared modules
    template <>
    struct ModuleTypeDeducer<SharedAnalyzer> {
      static constexpr auto value{ModuleType::analyzer};
    };

    template <>
    struct ModuleTypeDeducer<SharedFilter> {
      static constexpr auto value{ModuleType::filter};
    };

    template <>
    struct ModuleTypeDeducer<SharedOutputModule> {
      static constexpr auto value{ModuleType::output_module};
    };

    template <>
    struct ModuleTypeDeducer<SharedProducer> {
      static constexpr auto value{ModuleType::producer};
    };

    // Replicated modules
    template <>
    struct ModuleTypeDeducer<ReplicatedAnalyzer> {
      static constexpr auto value{ModuleType::analyzer};
    };

    template <>
    struct ModuleTypeDeducer<ReplicatedFilter> {
      static constexpr auto value{ModuleType::filter};
    };

    template <>
    struct ModuleTypeDeducer<ReplicatedOutputModule> {
      static constexpr auto value{ModuleType::output_module};
    };

    template <>
    struct ModuleTypeDeducer<ReplicatedProducer> {
      static constexpr auto value{ModuleType::producer};
    };

    // Only specializations allowed so that errors can be caught at
    // compile-time and not run-time.
    template <typename T>
    struct ModuleThreadingTypeDeducer;

    // Legacy modules
    template <>
    struct ModuleThreadingTypeDeducer<EDAnalyzer> {
      static constexpr auto value{ModuleThreadingType::legacy};
    };

    template <>
    struct ModuleThreadingTypeDeducer<EDFilter> {
      static constexpr auto value{ModuleThreadingType::legacy};
    };

    template <>
    struct ModuleThreadingTypeDeducer<OutputModule> {
      static constexpr auto value{ModuleThreadingType::legacy};
    };

    template <>
    struct ModuleThreadingTypeDeducer<EDProducer> {
      static constexpr auto value{ModuleThreadingType::legacy};
    };

    // Shared modules
    template <>
    struct ModuleThreadingTypeDeducer<SharedAnalyzer> {
      static constexpr auto value{ModuleThreadingType::shared};
    };

    template <>
    struct ModuleThreadingTypeDeducer<SharedFilter> {
      static constexpr auto value{ModuleThreadingType::shared};
    };

    template <>
    struct ModuleThreadingTypeDeducer<SharedOutputModule> {
      static constexpr auto value{ModuleThreadingType::shared};
    };

    template <>
    struct ModuleThreadingTypeDeducer<SharedProducer> {
      static constexpr auto value{ModuleThreadingType::shared};
    };

    // Replicated modules
    template <>
    struct ModuleThreadingTypeDeducer<ReplicatedAnalyzer> {
      static constexpr auto value{ModuleThreadingType::replicated};
    };

    template <>
    struct ModuleThreadingTypeDeducer<ReplicatedFilter> {
      static constexpr auto value{ModuleThreadingType::replicated};
    };

    template <>
    struct ModuleThreadingTypeDeducer<ReplicatedOutputModule> {
      static constexpr auto value{ModuleThreadingType::replicated};
    };

    template <>
    struct ModuleThreadingTypeDeducer<ReplicatedProducer> {
      static constexpr auto value{ModuleThreadingType::replicated};
    };

  } // namespace detail
} // namespace art

#endif /* art_Framework_Core_detail_ModuleTypeDeducer_h */

// Local Variables:
// mode: c++
// End:
