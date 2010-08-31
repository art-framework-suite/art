#ifndef FWCore_PluginManager_PresenceMacros_h
#define FWCore_PluginManager_PresenceMacros_h

#include "art/Framework/PluginManager/PresenceFactory.h"
#include "art/Utilities/Presence.h"

#define DEFINE_FWK_PRESENCE(type) \
  DEFINE_EDM_PLUGIN (edm::PresencePluginFactory,type,#type)

#define DEFINE_ANOTHER_FWK_PRESENCE(type) \
  DEFINE_EDM_PLUGIN (edm::PresencePluginFactory,type,#type)

#endif
