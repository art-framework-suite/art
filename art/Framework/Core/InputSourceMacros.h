#ifndef art_Framework_Core_InputSourceMacros_h
#define art_Framework_Core_InputSourceMacros_h

#include "art/Framework/Core/InputSource.h"

#define DEFINE_ART_INPUT_SOURCE(klass) \
   extern "C" { \
      std::unique_ptr<art::InputSource> \
         make(fhicl::ParameterSet const &ps, \
              art::InputSourceDescription &desc) \
         { return std::unique_ptr<art::InputSource>(new klass(ps, desc)); } \
   }

#endif /* art_Framework_Core_InputSourceMacros_h */

// Local Variables:
// mode: c++
// End:
