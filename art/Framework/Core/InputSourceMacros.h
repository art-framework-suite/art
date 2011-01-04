#ifndef Framework_InputSourceMacros_h
#define Framework_InputSourceMacros_h

#include "art/Framework/Core/InputSource.h"

#define DEFINE_ART_INPUT_SOURCE(klass) \
   extern "C" { \
      std::auto_ptr<art::InputSource> \
         make(fhicl::ParameterSet const &ps, \
              art::InputSourceDescription const &desc) \
         { return std::auto_ptr<art::InputSource>(new klass(ps, desc)); } \
   }

#endif
