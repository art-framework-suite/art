#ifndef art_Framework_Art_detail_md_collector_MetadataCollectorForModule_h
#define art_Framework_Art_detail_md_collector_MetadataCollectorForModule_h

#include "art/Framework/Art/detail/MetadataCollector.h"
#include "art/Framework/Art/detail/MetadataRegexHelpers.h"
#include "art/Framework/Art/detail/PrintFormatting.h"
#include "art/Framework/Art/detail/PluginMetadata.h"
#include "art/Utilities/PluginSuffixes.h"
#include "art/Utilities/bold_fontify.h"

#include <regex>

namespace art {
  namespace detail {

    template<>
    class MetadataCollectorFor<suffix_type::module> : public MetadataCollector {
    public:

      PluginMetadata doCollect(LibraryInfo const& li) const override
      {
        return {header_(li), details_(li), allowed_configuration_(li)};
      }

    private:
      std::string header_(LibraryInfo const& li) const
      {
        std::ostringstream result;
        std::string const long_spec = li.long_spec().empty() ? " [ No alternate specification available ] " : li.long_spec();
        result << indent_1()
               << "module_type : " << bold_fontify(li.short_spec())
               << " (or \"" << long_spec << "\")"
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
        replace_type(printedConfig, li.short_spec(), regex_for_spec("module_type"));

        result << printedConfig;
        return result.str();
      }
    };

  }
}

#endif /* art_Framework_Art_detail_md_collector_MetadataCollectorForModule_h */

// Local variables:
// mode: c++
// End:
