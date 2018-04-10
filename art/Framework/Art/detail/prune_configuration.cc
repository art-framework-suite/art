#include "art/Framework/Art/detail/exists_outside_prolog.h"
#include "art/Framework/Art/detail/prune_configuration.h"
#include "fhiclcpp/extended_value.h"

#include <initializer_list>
#include <iostream>
#include <set>

using namespace fhicl;

namespace {
  // void
  // prune_physics_configuration(extended_value& physics)
  // {
  //   if (!physics.is_a(TABLE)) {
  //     return;
  //   }
  //   auto& tbl = boost::any_cast<extended_value::table_t&>(physics.value);
  //   std::vector<std::string> modules;
  //   std::vector<std::string> paths;
  //   for (auto& pr : tbl) {
  //     std::cerr << pr.first << '\n';
  //   }
  // }

  std::set<std::string>
  declared_modules(intermediate_table const& config,
                   std::initializer_list<char const*> tables)
  {
    std::set<std::string> result;
    for (auto const tbl : tables) {
      if (!art::detail::exists_outside_prolog(config, tbl)) continue;

    }
    return result;
  }


}

void
art::detail::prune_configuration(intermediate_table& config)
{
  auto modifiers = declared_modules(config, {"physics.producers", "physics.filters"});
  auto observers = declared_modules(config, {"physics.analyzers", "outputs"});
  //  auto* physics_table = raw_config.locate("physics");
}
