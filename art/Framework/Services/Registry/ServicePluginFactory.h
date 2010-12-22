#ifndef ServiceRegistry_ServicePluginFactory_h
#define ServiceRegistry_ServicePluginFactory_h

//
// Package:     ServiceRegistry
// Class  :     ServicePluginFactory
//

#include "art/Framework/PluginManager/PluginFactory.h"

namespace art {
   namespace serviceregistry {
      class ServiceMakerBase;

     typedef artplugin::PluginFactory< ServiceMakerBase* ()> ServicePluginFactory;
   }
}

#endif  // ServiceRegistry_ServicePluginFactory_h
