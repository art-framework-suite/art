#include "art/Framework/Art/detail/PluginSymbolResolvers.h"

namespace bfs = boost::filesystem;
using cet::LibraryManager;

namespace art {
  namespace detail {

    template <>
    std::string getType<suffix_type::module>(cet::LibraryManager const& lm,
                                             std::string const& fullSpec)
    {
      using ModuleTypeFunc_t = art::ModuleType();

      auto type = [&lm,&fullSpec](){
        ModuleTypeFunc_t* symbolType{};
        lm.getSymbolByLibspec(fullSpec, "moduleType", symbolType);
        return art::to_string(symbolType());
      };

      return resolve_if_present(type, __func__, "[ error ]");
    }

    template <>
    std::string getType<suffix_type::plugin>(cet::LibraryManager const& lm,
                                             std::string const& fullSpec)
    {
      using PluginTypeFunc_t = std::string();

      auto type = [&lm,&fullSpec](){
        PluginTypeFunc_t* symbolType{};
        lm.getSymbolByLibspec(fullSpec, "pluginType", symbolType);
        return symbolType();
      };

      return resolve_if_present(type, __func__, "[ error ]");
    }
  }
}
