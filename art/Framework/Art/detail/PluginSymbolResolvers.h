#ifndef art_Framework_Art_detail_PluginSymbolResolvers_h
#define art_Framework_Art_detail_PluginSymbolResolvers_h

#include "art/Framework/Art/detail/PrintFormatting.h"
#include "art/Framework/Core/ModuleType.h"
#include "art/Utilities/PluginSuffixes.h"
#include "boost/filesystem.hpp"
#include "cetlib/LibraryManager.h"

#include <iostream>
#include <sstream>
#include <string>

namespace art {
  namespace detail {

    template <typename F>
    std::string resolve_if_present(F f, std::string const& caller, std::string result)
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

      auto path = [&lm,&fullspec](){
        GetSourceLoc_t* symbolLoc{};
        lm.getSymbolByLibspec(fullspec, "get_source_location", symbolLoc);
        std::string source {symbolLoc()};
        boost::filesystem::path const p {source};
        if (!boost::filesystem::exists(p)) {
          source = "/ [ external source ] /"+fullspec+"_"+Suffixes::get(st)+".cc";
        }
        return source;
      };

      return resolve_if_present(path, __func__, "[ not found ]");
    }

    template <art::suffix_type>
    std::string getType(cet::LibraryManager const&,
                        std::string const& /*fullSpec*/)
    {
      return {};
    }

    template <> std::string getType<suffix_type::module>(cet::LibraryManager const& lm, std::string const& fullSpec);
    template <> std::string getType<suffix_type::plugin>(cet::LibraryManager const& lm, std::string const& fullSpec);

    template <art::suffix_type>
    std::string getDescription(cet::LibraryManager const& lm,
                               std::string const& fullSpec,
                               std::string const& name,
                               std::string const& tab)
    {
      using GetDescription_t = std::ostream&(std::ostream&, std::string const&, std::string const&);

      auto description = [&lm, &fullSpec, &name, &tab]() {
        GetDescription_t* symbolType{};
        lm.getSymbolByLibspec(fullSpec, "print_description", symbolType);
        std::ostringstream oss;
        symbolType(oss, name, tab);
        return oss.str();
      };

      return resolve_if_present(description, __func__, "");
    }

  }
}

#endif /* art_Framework_Art_detail_PluginSymbolResolvers_h */

// Local variables:
// mode: c++
// End:
