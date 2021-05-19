#ifndef art_Framework_Core_detail_consumed_products_h
#define art_Framework_Core_detail_consumed_products_h

#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Principal/ConsumesInfo.h"
#include "art/Framework/Principal/fwd.h"

#include <map>
#include <set>
#include <string>

namespace art::detail {
  using config_const_iterator =
    std::vector<WorkerInPath::ConfigInfo>::const_iterator;

  std::set<ProductInfo> consumed_products_for_module(
    std::string const& current_process,
    ConsumesInfo::consumables_t::mapped_type const& consumables,
    std::map<std::string, std::set<ProductInfo>> const& produced_products,
    std::map<std::string, std::set<std::string>> const& viewable_products,
    config_const_iterator const config_begin,
    config_const_iterator const config_it);
}

#endif /* art_Framework_Core_detail_consumed_products_h */

// Local Variables:
// mode: c++
// End:
