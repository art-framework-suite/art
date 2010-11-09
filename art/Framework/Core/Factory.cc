#include "art/Framework/Core/Factory.h"

#include "art/Utilities/DebugMacros.h"
#include "art/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include <iostream>


namespace
{
  std::string translate_typespec_to_libname(std::string const& str)

  {
    // TODO: consider using std::tranlate and a lambda function ...
    std::string result(str);

    for (std::string::iterator i = result.begin(), e = result.end();
	 i != e; ++i)
      {
	if (*i == '/') *i = '_';
      }
  }
}

namespace art
{

  Factory::~Factory()
  {
  }

  Factory&
  Factory::the_instance_()
  {
    static Factory me;
    return me;
  }

  Factory::Factory() :
    libman_("plugin")
  {
  }

  std::auto_ptr<Worker> Factory::makeWorker(WorkerParams const& p,
					    ModuleDescription const& md)
  {
    typedef Worker* (*factor_fcn_t)(WorkerParams const&, ModuleDescription const&);
    Factory& me = the_instance_();

    std::string modtype(p.pset_->get<std::string>("_module_type"));
    std::string libname = translate_typespec_to_libname(modtype);
    factory_fcn_t* symbol = (factory_fcn_t*)libman_.getSymbol(libname, "make_temp");
    if (symbol == 0)
      throw art::Exception(errors::Configuration,"UnknownModule")
	<< "Module " << modtype
	<< " with version " << p.releaseVersion_
	<< " was not registered.\n"
	<< "Perhaps your module type is misspelled or is not a "
	<< "framework plugin.";

    return (*symbol)(p,md);

  }

}
