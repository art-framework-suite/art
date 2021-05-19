#ifndef art_Framework_Core_detail_ModuleGraphInfo_h
#define art_Framework_Core_detail_ModuleGraphInfo_h

#include "art/Framework/Principal/ProductInfo.h"
#include "art/Persistency/Provenance/ModuleType.h"

#include <iosfwd>
#include <set>
#include <string>

namespace art::detail {
  struct ModuleGraphInfo {
    ModuleType module_type{ModuleType::non_art};
    std::set<ProductInfo> produced_products{};
    std::set<ProductInfo> consumed_products{};
    // 'select_events' is only used for analyzers and output
    // modules.
    std::set<std::string> select_events{};
    std::set<std::string> paths{};
  };

  std::ostream& operator<<(std::ostream& os, ModuleGraphInfo const& info);
}

#endif /* art_Framework_Core_detail_ModuleGraphInfo_h */

// Local Variables:
// mode: c++
// End:
