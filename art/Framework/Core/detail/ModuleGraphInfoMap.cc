#include "art/Framework/Core/detail/ModuleGraphInfoMap.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"

#include <algorithm>

using art::detail::collection_map_t;
using art::detail::collection_t;
using art::detail::module_name_t;
using art::detail::name_set_t;
using art::detail::names_t;

namespace {
  auto
  map_to_vec(collection_map_t const& modules)
  {
    collection_t tmp;
    cet::copy_all(modules, back_inserter(tmp));
    return tmp;
  }
}

art::detail::ModuleGraphInfoMap::ModuleGraphInfoMap(
  collection_map_t const& modules)
  : modules_{map_to_vec(modules)}
  , begin_{cbegin(modules_)}
  , end_{cend(modules_)}
{}

auto
art::detail::ModuleGraphInfoMap::vertex_index(std::string const& name) const
  -> distance_t
{
  auto const it = std::find_if(
    begin_, end_, [&name](auto const& pr) { return pr.first == name; });
  if (it == end_) {
    throw art::Exception{art::errors::LogicError}
      << "The module name '" << name << "' does not correspond to an element\n"
      << "in the set of filters and producers used to create the workflow "
         "graph.\n";
  }
  return std::distance(begin_, it);
}

auto
art::detail::ModuleGraphInfoMap::info(std::string const& name) const
  -> ModuleGraphInfo const&
{
  return modules_[vertex_index(name)].second;
}

std::ostream&
art::detail::operator<<(std::ostream& os, ModuleGraphInfo const& info)
{
  os << "Module type: " << to_string(info.module_type) << '\n';
  os << "Product dependencies: ";
  for (auto const& dep : info.product_dependencies) {
    os << dep << ' ';
  }
  os << "\nPaths: ";
  for (auto const& path : info.paths) {
    os << path << ' ';
  }
  return os;
}
