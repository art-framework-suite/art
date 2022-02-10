#include "art/Framework/Core/InputSourceFactory.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/InputSource.h"
#include "art/Utilities/PluginSuffixes.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/LibraryManager.h"
#include "cetlib/detail/wrapLibraryManagerException.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>
#include <string>

namespace art::InputSourceFactory {

  std::unique_ptr<InputSource>
  make(fhicl::ParameterSet const& conf, InputSourceDescription& desc)
  {
    auto const libspec = conf.get<std::string>("module_type");
    FDEBUG(1) << "InputSourceFactory: module_type = " << libspec << '\n';
    using make_t = std::unique_ptr<InputSource>(fhicl::ParameterSet const&,
                                                InputSourceDescription&);
    make_t* symbol = nullptr;
    try {
      cet::LibraryManager lm_{Suffixes::source()};
      lm_.getSymbolByLibspec(libspec, "make", symbol);
    }
    catch (Exception const& e) {
      cet::detail::wrapLibraryManagerException(
        e, "InputSource", libspec, getReleaseVersion());
    }
    if (symbol == nullptr) {
      throw Exception(errors::Configuration, "BadPluginLibrary")
        << "InputSource " << libspec
        << " has internal symbol definition problems: consult an expert.";
    }
    auto wm = symbol(conf, desc);
    FDEBUG(1) << "InputSourceFactory: created input source " << libspec << '\n';
    return wm;
  }

} // namespace art::InputSourceFactory
