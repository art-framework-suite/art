#ifndef art_Framework_Art_detail_md_collector_MetadataCollectorForModule_h
#define art_Framework_Art_detail_md_collector_MetadataCollectorForModule_h

#include "art/Framework/Art/detail/MetadataCollector.h"
#include "art/Framework/Art/detail/PluginMetadata.h"
#include "art/Framework/Art/detail/PrintFormatting.h"
#include "art/Framework/Art/detail/md-collector/print_description_blocks.h"
#include "art/Utilities/PluginSuffixes.h"

namespace art::detail {

  template <>
  class MetadataCollectorFor<suffix_type::module> : public MetadataCollector {
  public:
    PluginMetadata
    doCollect(LibraryInfo const& li,
              std::string const& prefix,
              std::string const& header_label,
              std::string const& param_to_replace) const override
    {
      return {print_header(li, header_label),
              details_(li),
              print_allowed_configuration(li, prefix, param_to_replace)};
    }

  private:
    std::string
    details_(LibraryInfo const& li) const
    {
      std::ostringstream result;
      result << indent__2() << "provider: " << li.provider() << '\n'
             << indent__2() << "type    : " << li.plugin_type() << '\n'
             << indent__2() << "source  : " << li.path() << '\n'
             << indent__2() << "library : " << li.so_name() << "\n\n";
      return result.str();
    }
  };
}

#endif /* art_Framework_Art_detail_md_collector_MetadataCollectorForModule_h   \
        */

// Local variables:
// mode: c++
// End:
