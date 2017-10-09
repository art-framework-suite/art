#ifndef art_Framework_Core_detail_ModuleTypeDeducer_h
#define art_Framework_Core_detail_ModuleTypeDeducer_h

////////////////////////////////////////////////////////////////////////
// ModuleTypeDeducer
//
// Used by ModuleMacros.h to ascertain the correct ModuleType for a
// module.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/ModuleType.h"

namespace art {
  namespace detail {
    // ModuleTypeDeducer.
    template <typename T>
    struct ModuleTypeDeducer;
    template <>
    struct ModuleTypeDeducer<art::EDAnalyzer>;
    template <>
    struct ModuleTypeDeducer<art::EDFilter>;
    template <>
    struct ModuleTypeDeducer<art::OutputModule>;
    template <>
    struct ModuleTypeDeducer<art::EDProducer>;
  }
}

template <typename T>
struct art::detail::ModuleTypeDeducer {
  static constexpr ModuleType value = ModuleType::NON_ART;
};

template <>
struct art::detail::ModuleTypeDeducer<art::EDAnalyzer> {
  static constexpr ModuleType value = ModuleType::ANALYZER;
};

template <>
struct art::detail::ModuleTypeDeducer<art::EDFilter> {
  static constexpr ModuleType value = ModuleType::FILTER;
};

template <>
struct art::detail::ModuleTypeDeducer<art::OutputModule> {
  static constexpr ModuleType value = ModuleType::OUTPUT;
};

template <>
struct art::detail::ModuleTypeDeducer<art::EDProducer> {
  static constexpr ModuleType value = ModuleType::PRODUCER;
};
#endif /* art_Framework_Core_detail_ModuleTypeDeducer_h */

// Local Variables:
// mode: c++
// End:
