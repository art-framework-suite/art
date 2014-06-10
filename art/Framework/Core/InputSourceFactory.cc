#include "art/Framework/Core/InputSourceFactory.h"

#include "cetlib/detail/wrapLibraryManagerException.h"
#include "art/Utilities/DebugMacros.h"
#include "art/Utilities/Exception.h"
#include "art/Version/GetReleaseVersion.h"
#include "fhiclcpp/ParameterSet.h"
#include <iostream>

using fhicl::ParameterSet;
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

std::unique_ptr<InputSource>
InputSourceFactory::make(ParameterSet const& conf,
                        InputSourceDescription & desc)
{
   std::string libspec = conf.get<std::string>("module_type");

   FDEBUG(1) << "InputSourceFactory: module_type = " << libspec << std::endl;

   typedef std::unique_ptr<InputSource>
      (make_t)(fhicl::ParameterSet const&,
               InputSourceDescription &);

   make_t *symbol = nullptr;

   try {
     the_factory_().lm_.getSymbolByLibspec(libspec, "make", symbol);
   }
   catch (art::Exception const &e) {
     cet::detail::wrapLibraryManagerException(e, "InputSource", libspec, getReleaseVersion());
   }
   if (symbol == nullptr) {
      throw art::Exception(errors::Configuration, "BadPluginLibrary")
         << "InputSource " << libspec
         << " has internal symbol definition problems: consult an expert.";
   }
   std::unique_ptr<InputSource> wm = symbol(conf, desc);

   FDEBUG(1) << "InputSourceFactory: created input source "
             << libspec
             << std::endl;

   return wm;
}
