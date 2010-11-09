#include "art/Framework/Core/InputSourceFactory.h"

#include "art/Utilities/DebugMacros.h"
#include "art/Utilities/Exception.h"

#include "fhiclcpp/ParameterSet.h"
  using fhicl::ParameterSet;

#include <iostream>


EDM_REGISTER_PLUGINFACTORY(art::InputSourcePluginFactory,"CMS EDM Framework InputSource");

namespace art {

  InputSourceFactory::~InputSourceFactory()
  { }

  InputSourceFactory::InputSourceFactory()
  { }

  InputSourceFactory InputSourceFactory::singleInstance_;

  InputSourceFactory* InputSourceFactory::get()
  {
    // will not work with plugin factories
    //static InputSourceFactory f;
    //return &f;

    return &singleInstance_;
  }

  std::auto_ptr<InputSource>
  InputSourceFactory::makeInputSource(ParameterSet const& conf,
                                      InputSourceDescription const& desc) const
  {
    std::string modtype = conf.get<std::string>("@module_type");
    FDEBUG(1) << "InputSourceFactory: module_type = " << modtype << std::endl;
    std::auto_ptr<InputSource> wm;
    try {
      wm = std::auto_ptr<InputSource>(InputSourcePluginFactory::get()->create(modtype,conf,desc));
    }
    catch(art::Exception& ex) {
      ex << "Error occurred while creating source " << modtype << "\n";
      throw ex;
    }
    catch(cet::exception& e) {
      e << "Error occurred while creating source " << modtype << "\n";
      throw e;
    }

    if(wm.get()==0) {
        throw art::Exception(errors::Configuration,"NoSourceModule")
          << "InputSource Factory:\n"
          << "Cannot find source type from ParameterSet: "
          << modtype << "\n"
          << "Perhaps your source type is misspelled or is not an EDM Plugin?\n"
          << "Try running EdmPluginDump to obtain a list of available Plugins.";
    }

    wm->registerProducts();

    FDEBUG(1) << "InputSourceFactory: created input source "
              << modtype
              << std::endl;

    return wm;
  }

}  // namespace art
