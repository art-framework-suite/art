#ifndef art_Framework_Art_detail_md_collector_MetadataCollectorForService_h
#define art_Framework_Art_detail_md_collector_MetadataCollectorForService_h

#include "art/Framework/Art/detail/MetadataCollector.h"
#include "art/Framework/Art/detail/PluginMetadata.h"
#include "art/Framework/Art/detail/PrintFormatting.h"
#include "art/Framework/Art/detail/md-collector/print_description_blocks.h"
#include "art/Utilities/PluginSuffixes.h"
#include "cetlib/bold_fontify.h"

#include <regex>
#include <string>

namespace art::detail {

  template <>
  class MetadataCollectorFor<suffix_type::service> : public MetadataCollector {
  public:
    PluginMetadata
    doCollect(LibraryInfo const& li,
              std::string const& prefix,
              std::string const& header_label [[maybe_unused]],
              std::string const& param_to_replace) const override
    {
      return {header_(li),
              details_(li),
              print_allowed_configuration(li, prefix, param_to_replace)};
    }

  private:
    std::string
    header_(LibraryInfo const& li) const
    {
      std::string const& printed_name = li.short_spec();
      std::ostringstream result;
      result << indent_1() << "service : " << cet::bold_fontify(printed_name)
             << "\n\n";
      return result.str();
    }

    std::string
    details_(LibraryInfo const& li) const
    {
      std::ostringstream result;
      result << indent__2() << "provider: " << li.provider() << "\n"
             << indent__2() << "source  : " << li.path() << "\n"
             << indent__2() << "library : " << li.so_name() << "\n\n";
      return result.str();
    }
  };

}

#endif /* art_Framework_Art_detail_md_collector_MetadataCollectorForService_h */

// Local variables:
// mode: c++
// End:
