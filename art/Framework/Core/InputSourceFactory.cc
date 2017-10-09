#include "art/Framework/Core/InputSourceFactory.h"

#include "art/Version/GetReleaseVersion.h"
#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/detail/wrapLibraryManagerException.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

using fhicl::ParameterSet;
using namespace art;

InputSourceFactory&
InputSourceFactory::instance()
{
  static InputSourceFactory the_factory;
  return the_factory;
}

std::unique_ptr<InputSource>
InputSourceFactory::make(ParameterSet const& conf, InputSourceDescription& desc)
{
  auto const& libspec = conf.get<std::string>("module_type");

  FDEBUG(1) << "InputSourceFactory: module_type = " << libspec << std::endl;

  using make_t = std::unique_ptr<InputSource>(fhicl::ParameterSet const&,
                                              InputSourceDescription&);

  make_t* symbol = nullptr;

  try {
    instance().lm_.getSymbolByLibspec(libspec, "make", symbol);
  }
  catch (art::Exception const& e) {
    cet::detail::wrapLibraryManagerException(
      e, "InputSource", libspec, getReleaseVersion());
  }
  if (symbol == nullptr) {
    throw art::Exception(errors::Configuration, "BadPluginLibrary")
      << "InputSource " << libspec
      << " has internal symbol definition problems: consult an expert.";
  }
  auto wm = symbol(conf, desc);
  FDEBUG(1) << "InputSourceFactory: created input source " << libspec
            << std::endl;
  return wm;
}
