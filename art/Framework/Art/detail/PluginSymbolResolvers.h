#ifndef art_Framework_Art_detail_PluginSymbolResolvers_h
#define art_Framework_Art_detail_PluginSymbolResolvers_h

#include "fhiclcpp/types/ConfigurationTable.h"

#include <string>

namespace cet {
  class LibraryManager;
}

namespace art::detail {

  std::string getFilePath(cet::LibraryManager const& lm,
                          std::string const& fullspec);

  std::string getType(cet::LibraryManager const&,
                      std::string const& /*fullSpec*/);

  std::unique_ptr<fhicl::ConfigurationTable> getAllowedConfiguration(
    cet::LibraryManager const& lm,
    std::string const& fullSpec,
    std::string const& name);
} // namespace art::detail

#endif /* art_Framework_Art_detail_PluginSymbolResolvers_h */

// Local variables:
// mode: c++
// End:
