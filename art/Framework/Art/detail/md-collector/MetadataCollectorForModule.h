#ifndef art_Framework_Art_detail_MetadataCollectorForModule_h
#define art_Framework_Art_detail_MetadataCollectorForModule_h

#include "art/Framework/Art/detail/MetadataCollector.h"
#include "art/Framework/Art/detail/MetadataRegexHelpers.h"
#include "art/Framework/Art/detail/PrintFormatting.h"
#include "art/Framework/Art/detail/PluginMetadata.h"
#include "art/Utilities/PluginSuffixes.h"

#include <regex>

namespace art {
  namespace detail {

    template<>
    class MetadataCollectorFor<suffix_type::module> : public MetadataCollector {
    public:

      PluginMetadata doCollect(LibraryInfo const& li) const override
      {
        return { header_(li), details_(li), allowed_configuration_(li) };
      }

    private:
      std::string header_(LibraryInfo const& li) const
      {
        std::ostringstream result;
        result << indent_1()
               << "module_type : " << font_bold(li.short_spec())
               << " (or \"" << li.long_spec() << "\")"
               << "\n\n";
        return result.str();
      }

      std::string details_(LibraryInfo const& li) const
      {
        std::ostringstream result;
        result << indent__2() << "provider: " << li.provider()    << "\n"
               << indent__2() << "type    : " << li.plugin_type() << "\n"
               << indent__2() << "source  : " << li.path()        << "\n"
               << indent__2() << "library : " << li.so_name()     << "\n\n";
        return result.str();
      }

      std::string allowed_configuration_(LibraryInfo const& li) const
      {
        std::ostringstream result;
        result << indent_1()  << "Allowed configuration\n"
               << indent_1()  << "---------------------\n";

        std::string printedConfig = li.description();
        replace_module_type ( printedConfig, li.short_spec() );
        replace_label( "module_label", printedConfig );

        result << printedConfig;
        return result.str();
      }
    };

  }
}

#endif

// Local variables:
// mode: c++
// End:
