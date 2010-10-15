#ifndef FWCore_PluginManager_PresenceFactory_h
#define FWCore_PluginManager_PresenceFactory_h

#include "art/Utilities/Presence.h"
#include "art/Framework/PluginManager/PluginFactory.h"

#include <string>
#include <memory>

namespace art {
  typedef edmplugin::PluginFactory<Presence* ()> PresencePluginFactory;

  typedef Presence* (PresenceFunc)();

  class PresenceFactory {
  public:
    ~PresenceFactory();

    static PresenceFactory* get();

    std::auto_ptr<Presence>
      makePresence(std::string const & presence_type) const;

  private:
    PresenceFactory();
    //static PresenceFactory singleInstance_;
  };
}
#endif // FWCore_PluginManager_PresenceFactory_h
