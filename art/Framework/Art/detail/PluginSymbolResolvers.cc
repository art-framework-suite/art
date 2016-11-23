#include "art/Framework/Art/detail/PluginSymbolResolvers.h"

using namespace std::string_literals;
namespace bfs = boost::filesystem;
using cet::LibraryManager;


namespace art {
  namespace detail {

    template <>
    std::string getType<suffix_type::module>(cet::LibraryManager const& lm,
                                             std::string const& fullSpec)
    {
      using ModuleTypeFunc_t = art::ModuleType();

      auto type = [&lm,&fullSpec] {
        ModuleTypeFunc_t* symbolType {nullptr};
        lm.getSymbolByLibspec(fullSpec, "moduleType", symbolType);
        return art::to_string(symbolType());
      };

      return resolve_if_present(type, __func__, "[ error ]"s);
    }

    template <>
    std::string getType<suffix_type::plugin>(cet::LibraryManager const& lm,
                                             std::string const& fullSpec)
    {
      using PluginTypeFunc_t = std::string();

      auto type = [&lm,&fullSpec] {
        PluginTypeFunc_t* symbolType {nullptr};
        lm.getSymbolByLibspec(fullSpec, "pluginType", symbolType);
        return symbolType();
      };

      return resolve_if_present(type, __func__, "[ error ]"s);
    }

    template <>
    std::string getType<suffix_type::tool>(cet::LibraryManager const& lm,
                                           std::string const& fullSpec)
    {
      using ToolTypeFunc_t = std::string();

      auto type = [&lm,&fullSpec](){
        ToolTypeFunc_t* symbolType {nullptr};
        lm.getSymbolByLibspec(fullSpec, "toolType", symbolType);
        return symbolType();
      };

      return resolve_if_present(type, __func__, "[ error ]");
    }
  }
}
