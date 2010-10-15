#ifndef FWCore_Sources_VectorInputSourceMacros_h
#define FWCore_Sources_VectorInputSourceMacros_h

#include "art/Framework/IO/Sources/VectorInputSourceFactory.h"
#include "art/Framework/IO/Sources/VectorInputSource.h"

#define DEFINE_FWK_VECTOR_INPUT_SOURCE(type) \
  DEFINE_EDM_PLUGIN (art::VectorInputSourcePluginFactory,type,#type)

#define DEFINE_ANOTHER_FWK_VECTOR_INPUT_SOURCE(type) \
  DEFINE_EDM_PLUGIN (art::VectorInputSourcePluginFactory,type,#type)

#endif
