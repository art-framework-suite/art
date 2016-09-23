#ifndef art_Utilities_Tool_h
#define art_Utilities_Tool_h

#include "art/Utilities/PluginSuffixes.h"
#include "art/Version/GetReleaseVersion.h"
#include "cetlib/LibraryManager.h"
#include "cetlib/detail/wrapLibraryManagerException.h"
#include "fhiclcpp/ParameterSet.h"

#include <functional>
#include <memory>
#include <type_traits>

namespace art {

  template <typename T>
  using ToolClassMaker_t = std::unique_ptr<T>(fhicl::ParameterSet const&);

  //  template <typename T>
  // struct Tool;

  template <typename T>
  std::enable_if_t<!std::is_function<T>::value, std::unique_ptr<T>>
  make_tool(fhicl::ParameterSet const& pset)
  {
    cet::LibraryManager lm {Suffixes::tool()};
    std::string const libspec {pset.get<std::string>("tool_type")};
    ToolClassMaker_t<T>* symbol {nullptr};
    try {
      lm.getSymbolByLibspec(libspec, "makeTool", symbol);
    }
    catch (art::Exception& e) {
      cet::detail::wrapLibraryManagerException(e, "Tool", libspec, getReleaseVersion());
    }
    if (symbol == nullptr) {
      throw art::Exception(errors::Configuration, "BadPluginLibrary")
        << "Tool " << libspec
        << " with version " << getReleaseVersion()
        << " has internal symbol definition problems: consult an expert.";
    }
    return symbol(pset);
  }

  template <typename T>
  std::enable_if_t<std::is_function<T>::value, std::function<T>>
  make_tool(fhicl::ParameterSet const& pset)
  {
    cet::LibraryManager lm {Suffixes::tool()};
    std::string const libspec {pset.get<std::string>("tool_type")};
    T** symbol {nullptr};
    try {
      lm.getSymbolByLibspec(libspec, "toolFunction", symbol);
    }
    catch (art::Exception& e) {
      cet::detail::wrapLibraryManagerException(e, "Tool", libspec, getReleaseVersion());
    }
    if (symbol == nullptr) {
      throw art::Exception(errors::Configuration, "BadPluginLibrary")
        << "Tool " << libspec
        << " with version " << getReleaseVersion()
        << " has internal symbol definition problems: consult an expert.";
    }
    return *symbol;
  }

}

#endif

// Local variables:
// mode: c++
// End:
