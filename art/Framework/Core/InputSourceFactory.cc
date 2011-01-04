#include "art/Framework/Core/InputSourceFactory.h"

#include "art/Utilities/DebugMacros.h"
#include "art/Utilities/Exception.h"

#include "fhiclcpp/ParameterSet.h"
  using fhicl::ParameterSet;

#include <iostream>

using namespace art;

InputSourceFactory::InputSourceFactory()
   : lm_( "source" )
{ }

InputSourceFactory::~InputSourceFactory()
{ }

InputSourceFactory &
  InputSourceFactory::the_factory_()
{
  static InputSourceFactory the_factory;
  return the_factory;
}

std::auto_ptr<InputSource>
InputSourceFactory::makeInputSource(ParameterSet const& conf,
                                    InputSourceDescription const& desc)
{
   std::string libspec = conf.get<std::string>("module_type");

   FDEBUG(1) << "InputSourceFactory: module_type = " << libspec << std::endl;

   typedef std::auto_ptr<InputSource>
      (make_t)(fhicl::ParameterSet const&,
               InputSourceDescription const&);

   make_t *symbol = nullptr;

   try {
      // reinterpret_cast is required because void* can't be converted to a whole lot.
      symbol = reinterpret_cast<make_t*>( the_factory_().lm_.getSymbolByLibspec(libspec, "make") );
   }
   catch (cet::exception e) {
      throw art::Exception(errors::Configuration,"NoSourceModule", e)
         << "InputSource " << libspec
         << " was not registered.\n"
         << "Perhaps your module type is misspelled or is not a framework plugin.";
   }
   if (symbol == nullptr) {
      throw art::Exception(errors::Configuration, "BadPluginLibrary")
         << "InputSource " << libspec
         << " has internal symbol definition problems: consult an expert.";
   }
   std::auto_ptr<InputSource> wm = symbol(conf, desc);

   wm->registerProducts();

   FDEBUG(1) << "InputSourceFactory: created input source "
             << libspec
             << std::endl;

   return wm;
}
