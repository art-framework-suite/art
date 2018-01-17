#ifndef art_Framework_Core_InputSourceMacros_h
#define art_Framework_Core_InputSourceMacros_h

#include "art/Framework/Core/InputSource.h"
#include "cetlib/ProvideFilePathMacro.h"
#include "cetlib/compiler_macros.h"
#include "fhiclcpp/types/AllowedConfigurationMacro.h"

#define DEFINE_ART_INPUT_SOURCE(klass)                                  \
  EXTERN_C_FUNC_DECLARE_START                                           \
  CET_PROVIDE_FILE_PATH()                                               \
  FHICL_PROVIDE_ALLOWED_CONFIGURATION(klass)                            \
  std::unique_ptr<art::InputSource>                                     \
  make(fhicl::ParameterSet const& ps, art::InputSourceDescription& desc) \
  {                                                                     \
    return std::make_unique<klass>(ps, desc);                           \
  }                                                                     \
  EXTERN_C_FUNC_DECLARE_END

#endif /* art_Framework_Core_InputSourceMacros_h */

// Local Variables:
// mode: c++
// End:
