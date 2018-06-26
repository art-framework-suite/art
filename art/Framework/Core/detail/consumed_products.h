#ifndef art_Framework_Core_detail_consumed_products_h
#define art_Framework_Core_detail_consumed_products_h

#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Principal/Consumer.h"
#include "art/Framework/Principal/ProductInfo.h"

#include <map>
#include <set>
#include <string>

namespace art {
  namespace detail {
    using config_const_iterator =
      std::vector<WorkerInPath::ConfigInfo>::const_iterator;

    std::set<ProductInfo> consumed_products_for_module(
      std::string const& current_process,
      ConsumesInfo::consumables_t::mapped_type const& consumables,
      std::map<std::string, std::set<ProductInfo>> const& produced_products,
      std::map<std::string, std::set<std::string>> const& viewable_products,
      config_const_iterator const config_begin,
      config_const_iterator const config_it);

    bool product_match_found(
      std::map<std::string, std::set<ProductInfo>> const& produced_products,
      ProductInfo const& info);

    ProductInfo consumes_dependency(
      config_const_iterator const firstModuleOnPath,
      config_const_iterator const moduleConfig,
      ProductInfo const& prod_info,
      std::string const& current_process,
      std::map<std::string, std::set<ProductInfo>> const& produced_products);

    ProductInfo consumes_view_dependency(
      ProductInfo const& prod_info,
      std::string const& module_name,
      std::string const& current_process,
      std::map<std::string, std::set<std::string>> const& viewable_products);
  }
}

#endif /* art_Framework_Core_detail_consumed_products_h */

// Local Variables:
// mode: c++
// End:
