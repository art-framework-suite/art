#include "art/Framework/Art/detail/PluginSymbolResolvers.h"



namespace {
  // Function type aliases


}

namespace bfs = boost::filesystem;
using cet::LibraryManager;

std::string
art::detail::getModuleType( LibraryManager const& lm,
                            std::string const& fullSpec )
{
  std::string result {"[ error ]"};

  auto type = [&lm,&fullSpec](std::string& modType) {
    ModuleTypeFunc_t * symbolType{};
    lm.getSymbolByLibspec(fullSpec, "moduleType", symbolType);
    modType = art::to_string( symbolType() );
  };

  try_resolve(type, __func__, result);

  return result;
}

std::string
art::detail::getPluginType( LibraryManager const& lm,
                            std::string const& fullSpec )
{
  std::string result = "[ error ]";

  auto type = [&lm,&fullSpec](std::string& pluginType) {
    PluginTypeFunc_t * symbolType{};
    lm.getSymbolByLibspec(fullSpec, "pluginType", symbolType);
    pluginType = symbolType();
  };

  try_resolve(type, __func__, result);

  return result;
}
