#ifndef art_Framework_Art_detail_PluginSymbolResolvers_h
#define art_Framework_Art_detail_PluginSymbolResolvers_h

#include "art/Framework/Art/detail/PrintFormatting.h"
#include "art/Framework/Core/ModuleType.h"
#include "art/Utilities/ConfigurationTable.h"
#include "art/Utilities/PluginSuffixes.h"
#include "boost/filesystem.hpp"
#include "cetlib/LibraryManager.h"

#include <iostream>
#include <sstream>
#include <string>

namespace art {
  namespace detail {

    template <typename F, typename RT = decltype(std::declval<F>()())>
    RT resolve_if_present(F f, std::string const& caller, RT result)
    {
      try {
        result = f();
      }
      catch(cet::exception const& e) {
        std::cout << "In: " << caller << std::endl;
        std::cout << e.what() << std::endl;
      }
      return result;
    }

    template <art::suffix_type st>
    std::string getFilePath(cet::LibraryManager const& lm,
                            std::string const& fullspec)
    {
      using GetSourceLoc_t = std::string();

      using namespace std::string_literals;
      auto path = [&lm,&fullspec] {
        GetSourceLoc_t* symbolLoc{};
        lm.getSymbolByLibspec(fullspec, "get_source_location", symbolLoc);
        std::string source {symbolLoc()};
        boost::filesystem::path const p {source};
        if (!boost::filesystem::exists(p)) {
          source = "/ [ external source ] /"+fullspec+"_"+Suffixes::get(st)+".cc";
        }
        return source;
      };

      return resolve_if_present(path, __func__, "[ not found ]"s);
    }

    template <art::suffix_type>
    std::string getType(cet::LibraryManager const&,
                        std::string const& /*fullSpec*/)
    {
      return {};
    }

    template <> std::string getType<suffix_type::module>(cet::LibraryManager const& lm, std::string const& fullSpec);
    template <> std::string getType<suffix_type::plugin>(cet::LibraryManager const& lm, std::string const& fullSpec);
    template <> std::string getType<suffix_type::tool>(cet::LibraryManager const& lm, std::string const& fullSpec);

    template <art::suffix_type>
    std::unique_ptr<art::ConfigurationTable> getAllowedConfiguration(cet::LibraryManager const& lm,
                                                                     std::string const& fullSpec,
                                                                     std::string const& name)
    {
      using GetAllowedConfiguration_t = std::unique_ptr<art::ConfigurationTable>(std::string const&);

      auto description = [&lm, &fullSpec, &name] {
        GetAllowedConfiguration_t* symbolType{};
        lm.getSymbolByLibspec(fullSpec, "allowed_configuration", symbolType);
        return symbolType(name);
      };

      return resolve_if_present(description, __func__, std::unique_ptr<art::ConfigurationTable>{nullptr});
    }

  }
}

#endif /* art_Framework_Art_detail_PluginSymbolResolvers_h */

// Local variables:
// mode: c++
// End:
