#include "art/Framework/Art/detail/PluginSymbolResolvers.h"
#include "art/Persistency/Provenance/ModuleType.h"
#include "art/Utilities/PluginSuffixes.h"
#include "cetlib/LibraryManager.h"

#include <filesystem>
#include <iostream>

using namespace std::string_literals;
namespace fs = std::filesystem;
using cet::LibraryManager;

namespace {
  template <typename F, typename RT = decltype(std::declval<F>()())>
  RT
  resolve_if_present(F f, std::string const& caller, RT result)
  {
    try {
      result = f();
    }
    catch (cet::exception const& e) {
      std::cerr << "In: " << caller << '\n' << e.what() << '\n';
    }
    return result;
  }
}

namespace art::detail {

  std::string
  getFilePath(cet::LibraryManager const& lm, std::string const& fullspec)
  {
    using GetSourceLoc_t = std::string();

    using namespace std::string_literals;
    auto path = [&lm, &fullspec] {
      GetSourceLoc_t* symbolLoc{};
      lm.getSymbolByLibspec(fullspec, "get_source_location", symbolLoc);
      std::string source{symbolLoc()};
      fs::path const p{source};
      if (!fs::exists(p)) {
        source =
          "/ [ external source ] /" + fullspec + "_" + lm.libType() + ".cc";
      }
      return source;
    };

    return resolve_if_present(path, __func__, "[ not found ]"s);
  }

  std::string
  getType(cet::LibraryManager const& lm, std::string const& fullSpec)
  {
    auto const& suffix = lm.libType();
    if (suffix == Suffixes::module()) {
      using ModuleTypeFunc_t = art::ModuleType();

      auto type = [&lm, &fullSpec] {
        ModuleTypeFunc_t* symbolType{nullptr};
        lm.getSymbolByLibspec(fullSpec, "moduleType", symbolType);
        return to_string(symbolType());
      };

      return resolve_if_present(type, __func__, "[ error ]"s);
    }

    if (suffix == Suffixes::plugin()) {
      using PluginTypeFunc_t = std::string();

      auto type = [&lm, &fullSpec] {
        PluginTypeFunc_t* symbolType{nullptr};
        lm.getSymbolByLibspec(fullSpec, "pluginType", symbolType);
        return symbolType();
      };

      return resolve_if_present(type, __func__, "[ error ]"s);
    }

    if (suffix == Suffixes::tool()) {
      using ToolTypeFunc_t = std::string();

      auto type = [&lm, &fullSpec] {
        ToolTypeFunc_t* symbolType{nullptr};
        lm.getSymbolByLibspec(fullSpec, "toolType", symbolType);
        return symbolType();
      };

      return resolve_if_present(type, __func__, "[ error ]"s);
    }
    return {};
  }

  std::unique_ptr<fhicl::ConfigurationTable>
  getAllowedConfiguration(cet::LibraryManager const& lm,
                          std::string const& fullSpec,
                          std::string const& name)
  {
    using GetAllowedConfiguration_t =
      std::unique_ptr<fhicl::ConfigurationTable>(std::string const&);

    auto description = [&lm, &fullSpec, &name] {
      GetAllowedConfiguration_t* symbolType{};
      lm.getSymbolByLibspec(fullSpec, "allowed_configuration", symbolType);
      return symbolType(name);
    };

    return resolve_if_present(
      description,
      __func__,
      std::unique_ptr<fhicl::ConfigurationTable>{nullptr});
  }
} // namespace art::detail
