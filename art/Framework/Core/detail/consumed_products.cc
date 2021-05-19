#include "art/Framework/Core/detail/consumed_products.h"
#include "art/Framework/Core/detail/ModuleConfigInfo.h"
#include "art/Framework/Principal/ProductInfo.h"
#include "canvas/Utilities/Exception.h"

using namespace art;
using art::detail::config_const_iterator;

namespace {
  auto
  product_from_input_source(art::ProductInfo info)
  {
    info.label = "input_source";
    return info;
  }

  bool
  product_match_found(
    std::map<std::string, std::set<ProductInfo>> const& produced_products,
    ProductInfo const& info)
  {
    auto found = produced_products.find(info.label);
    if (found == cend(produced_products)) {
      return false;
    }
    // Do not use ProductInfo::operator< for the comparator as that
    // includes that process name, which is not required when
    // checking for a match in the current process.  If it were,
    // then only those consumes statements that included the current
    // process name (or 'current_process') would result in a match.
    return std::binary_search(
      cbegin(found->second),
      cend(found->second),
      info,
      [](auto const& a, auto const& b) {
        auto const& boundA = std::tie(a.friendlyClassName, a.label, a.instance);
        auto const& boundB = std::tie(b.friendlyClassName, b.label, b.instance);
        return boundA < boundB;
      });
  }

  ProductInfo
  consumes_dependency(
    config_const_iterator const firstModuleOnPath,
    config_const_iterator const moduleConfig,
    ProductInfo const& prod_info,
    std::string const& current_process,
    std::map<std::string, std::set<ProductInfo>> const& produced_products)
  {
    assert(prod_info.consumableType == ProductInfo::ConsumableType::Product);

    auto const mci = moduleConfig->moduleConfigInfo;
    assert(mci);

    // User has not specified the process name.
    if (prod_info.process.name().empty()) {
      if (!product_match_found(produced_products, prod_info)) {
        return product_from_input_source(prod_info);
      }

      // This is a necessary requirement if the consumes clause is on a
      // trigger path.
      if (is_modifier(mci->moduleType)) {
        auto found_on_path = std::find_if(
          firstModuleOnPath, moduleConfig, [&prod_info](auto const& config) {
            return config.moduleConfigInfo->modDescription.moduleLabel() ==
                   prod_info.label;
          });
        if (found_on_path == moduleConfig) {
          return product_from_input_source(prod_info);
        }
      }
      return prod_info;
    }

    // If we get here, the user has specified a process name.
    if (prod_info.process.name() != current_process) {
      return product_from_input_source(prod_info);
    }

    // The user has specified the current process name.
    if (product_match_found(produced_products, prod_info)) {
      return prod_info;
    }

    throw Exception{errors::Configuration}
      << "Module "
      << moduleConfig->moduleConfigInfo->modDescription.moduleLabel()
      << " expects to consume a product from module " << prod_info.label
      << " with the signature:\n"
      << "  Friendly class name: " << prod_info.friendlyClassName << '\n'
      << "  Instance name: " << prod_info.instance << '\n'
      << "  Process name: " << prod_info.process.name() << '\n'
      << "However, no product of that signature is provided by module "
      << prod_info.label << ".\n";
  }

  ProductInfo
  consumes_view_dependency(
    ProductInfo const& prod_info,
    std::string const& module_name,
    std::string const& current_process,
    std::map<std::string, std::set<std::string>> const& viewable_products)
  {
    assert(prod_info.consumableType ==
           ProductInfo::ConsumableType::ViewElement);

    // User has not specified the process name
    if (prod_info.process.name().empty()) {
      auto ml_found = viewable_products.find(prod_info.label);
      if (ml_found == cend(viewable_products)) {
        return product_from_input_source(prod_info);
      }

      auto prod_found = ml_found->second.find(prod_info.instance);
      if (prod_found == cend(ml_found->second)) {
        return product_from_input_source(prod_info);
      }

      // The correct module has been found for which a view can be
      // formed.
      return prod_info;
    }

    // If we get here, the user has specified a process name.
    if (prod_info.process.name() != current_process) {
      return product_from_input_source(prod_info);
    }

    // Current process
    Exception e{errors::Configuration,
                "An error occurred while checking data-product dependencies "
                "for this job.\n"};
    auto ml_found = viewable_products.find(prod_info.label);
    if (ml_found == cend(viewable_products)) {
      throw e << "Module " << module_name
              << " expects to consume a view of type from module "
              << prod_info.label << ".\n"
              << "However, module " << prod_info.label
              << " does not produce a product\n"
              << "for which a view can be formed.\n";
    }

    auto prod_found = ml_found->second.find(prod_info.instance);
    if (prod_found == cend(ml_found->second)) {
      throw e << "Module " << module_name
              << " expects to consume a view with the following signature:\n"
              << "  Module label: " << prod_info.label << '\n'
              << "  Instance name: " << prod_info.instance << '\n'
              << "However, module " << prod_info.label
              << " does not produce a product for which such a view "
                 "can be formed.\n";
    }
    return prod_info;
  }
}

std::set<ProductInfo>
detail::consumed_products_for_module(
  std::string const& current_process,
  ConsumesInfo::consumables_t::mapped_type const& consumables,
  std::map<std::string, std::set<ProductInfo>> const& produced_products,
  std::map<std::string, std::set<std::string>> const& viewable_products,
  config_const_iterator const config_begin,
  config_const_iterator const config_it)
{
  auto const& module_name =
    config_it->moduleConfigInfo->modDescription.moduleLabel();
  std::set<ProductInfo> result;
  for (auto const& per_branch_type : consumables) {
    for (auto const& prod_info : per_branch_type) {
      switch (prod_info.consumableType) {
      case ProductInfo::ConsumableType::Product: {
        auto dep = consumes_dependency(config_begin,
                                       config_it,
                                       prod_info,
                                       current_process,
                                       produced_products);
        result.insert(std::move(dep));
        break;
      }
      case ProductInfo::ConsumableType::Many: {
        // Loop through modules on this path, introducing
        // product-lookup dependencies if the type of the product
        // created by the module matches the type requested in the
        // consumesMany call.
        auto const& class_name = prod_info.friendlyClassName;
        for (auto mit = config_begin; mit != config_it; ++mit) {
          auto possible_products = produced_products.find(
            mit->moduleConfigInfo->modDescription.moduleLabel());
          if (possible_products == cend(produced_products)) {
            continue;
          }
          cet::copy_if_all(possible_products->second,
                           inserter(result, begin(result)),
                           [&class_name](auto const& pi) {
                             return class_name == pi.friendlyClassName;
                           });
        }
        break;
      }
      case ProductInfo::ConsumableType::ViewElement: {
        auto dep = consumes_view_dependency(
          prod_info, module_name, current_process, viewable_products);
        result.insert(std::move(dep));
      }
        // No default case to allow compiler to warn.
      }
    }
  }
  return result;
}
