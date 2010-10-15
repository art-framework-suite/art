#ifndef Framework_InputSourceMacros_h
#define Framework_InputSourceMacros_h

#include "art/Framework/Core/InputSourceFactory.h"
#include "art/Framework/Core/InputSource.h"

#define DEFINE_FWK_INPUT_SOURCE(type) \
  //DEFINE_EDM_PLUGIN (art::InputSourcePluginFactory,type,#type)

#define DEFINE_ANOTHER_FWK_INPUT_SOURCE(type) \
  //DEFINE_EDM_PLUGIN (art::InputSourcePluginFactory,type,#type)

#endif
