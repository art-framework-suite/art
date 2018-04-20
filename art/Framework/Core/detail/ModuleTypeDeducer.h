#ifndef art_Framework_Core_detail_ModuleTypeDeducer_h
#define art_Framework_Core_detail_ModuleTypeDeducer_h
// vim: set sw=2 expandtab :

//
//  Used by ModuleMacros.h to ascertain the correct
//  ModuleType for a module.
//

#include "art/Framework/Core/ModuleType.h"

namespace art {
  namespace shared {
    class Analyzer;
    class Producer;
    class Filter;
    class OutputModule;
  }
  namespace replicated {
    class Analyzer;
    class Producer;
    class Filter;
    class OutputModule;
  }

  namespace detail {

    template <typename T>
    struct ModuleTypeDeducer {
      static constexpr ModuleType value = ModuleType::non_art;
    };

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
    struct ModuleTypeDeducer<shared::Analyzer> {
      static constexpr ModuleType value = ModuleType::analyzer;
    };

    template <>
    struct ModuleTypeDeducer<shared::Filter> {
      static constexpr ModuleType value = ModuleType::filter;
    };

    template <>
    struct ModuleTypeDeducer<shared::OutputModule> {
      static constexpr ModuleType value = ModuleType::output_module;
    };

    template <>
    struct ModuleTypeDeducer<shared::Producer> {
      static constexpr ModuleType value = ModuleType::producer;
    };

    template <>
    struct ModuleTypeDeducer<replicated::Analyzer> {
      static constexpr ModuleType value = ModuleType::analyzer;
    };

    template <>
    struct ModuleTypeDeducer<replicated::Filter> {
      static constexpr ModuleType value = ModuleType::filter;
    };

    template <>
    struct ModuleTypeDeducer<replicated::OutputModule> {
      static constexpr ModuleType value = ModuleType::output_module;
    };

    template <>
    struct ModuleTypeDeducer<replicated::Producer> {
      static constexpr ModuleType value = ModuleType::producer;
    };

  } // namespace detail
} // namespace art

#endif /* art_Framework_Core_detail_ModuleTypeDeducer_h */

// Local Variables:
// mode: c++
// End:
