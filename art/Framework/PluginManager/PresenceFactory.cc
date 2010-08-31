#include "art/Framework/PluginManager/PresenceFactory.h"
#include "art/Utilities/EDMException.h"
#include "art/Utilities/DebugMacros.h"

#include <iostream>

EDM_REGISTER_PLUGINFACTORY(edm::PresencePluginFactory,"CMS EDM Framework Presence");

namespace edm {

  PresenceFactory::~PresenceFactory() {
  }

  PresenceFactory::PresenceFactory() {
  }


  PresenceFactory* PresenceFactory::get() {
    static PresenceFactory singleInstance_;
    return &singleInstance_;
  }

  std::auto_ptr<Presence>
  PresenceFactory::
  makePresence(std::string const & presence_type) const {
    std::auto_ptr<Presence> sp(PresencePluginFactory::get()->create(presence_type));

    if(sp.get()==0) {
	throw edm::Exception(errors::Configuration, "NoPresenceModule")
	  << "Presence Factory:\n"
	  << "Cannot find presence type: "
	  << presence_type << "\n"
	  << "Perhaps the name is misspelled or is not a Plugin?\n"
	  << "Try running EdmPluginDump to obtain a list of available Plugins.";
    }

    FDEBUG(1) << "PresenceFactory: created presence "
	      << presence_type
	      << std::endl;

    return sp;
  }
}

