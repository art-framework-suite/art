#ifndef art_Framework_Core_detail_ModuleTypeDeducer_h
#define art_Framework_Core_detail_ModuleTypeDeducer_h
// vim: set sw=2 expandtab :

//
//  Used by ModuleMacros.h to ascertain the correct
//  ModuleType for a module.
//

#include "art/Framework/Core/ModuleType.h"

namespace art {
  namespace detail {

    template <typename T>
    struct ModuleTypeDeducer {
      static constexpr ModuleType value = ModuleType::NON_ART;
    };

    template <>
    struct ModuleTypeDeducer<EDAnalyzer> {
      static constexpr ModuleType value = ModuleType::ANALYZER;
    };

    template <>
    struct ModuleTypeDeducer<EDFilter> {
      static constexpr ModuleType value = ModuleType::FILTER;
    };

    template <>
    struct ModuleTypeDeducer<OutputModule> {
      static constexpr ModuleType value = ModuleType::OUTPUT;
    };

    template <>
    struct ModuleTypeDeducer<EDProducer> {
      static constexpr ModuleType value = ModuleType::PRODUCER;
    };

  } // namespace detail
} // namespace art

#endif /* art_Framework_Core_detail_ModuleTypeDeducer_h */

// Local Variables:
// mode: c++
// End:
