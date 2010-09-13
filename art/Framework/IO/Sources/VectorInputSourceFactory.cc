#include "art/Framework/IO/Sources/VectorInputSourceFactory.h"
#include "art/Utilities/DebugMacros.h"
#include "art/Utilities/EDMException.h"

#include "fhiclcpp/ParameterSet.h"

#include <iostream>


EDM_REGISTER_PLUGINFACTORY(edm::VectorInputSourcePluginFactory,"CMS EDM Framework VectorInputSource");

namespace edm {

  VectorInputSourceFactory::~VectorInputSourceFactory()
  { }

  VectorInputSourceFactory::VectorInputSourceFactory()
  { }

  VectorInputSourceFactory VectorInputSourceFactory::singleInstance_;

  VectorInputSourceFactory* VectorInputSourceFactory::get()
  {
    // will not work with plugin factories
    //static InputSourceFactory f;
    //return &f;

    return &singleInstance_;
  }

  std::auto_ptr<VectorInputSource>
  VectorInputSourceFactory::makeVectorInputSource(fhicl::ParameterSet const& conf,
					InputSourceDescription const& desc) const

  {
    std::string modtype = conf.getString("@module_type");
    FDEBUG(1) << "VectorInputSourceFactory: module_type = " << modtype << std::endl;
    std::auto_ptr<VectorInputSource> wm(VectorInputSourcePluginFactory::get()->create(modtype,conf,desc));

    if(wm.get()==0)
      {
	throw edm::Exception(errors::Configuration,"NoSourceModule")
	  << "VectorInputSource Factory:\n"
	     "Cannot find source type from ParameterSet: "
	  << modtype << "\n"
	  << "Perhaps your source type is misspelled or is not an EDM Plugin?\n"
	     "Try running EdmPluginDump to obtain a list of available Plugins.";
      }

    FDEBUG(1) << "VectorInputSourceFactory: created input source "
	      << modtype
	      << std::endl;

    return wm;
  }

}  // namespace edm
