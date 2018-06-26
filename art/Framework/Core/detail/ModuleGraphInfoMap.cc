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
    throw Exception{
      errors::Configuration,
      "An error occurred while constructing the data-dependency graph.\n"}
      << "The module name '" << name << "' is not included in the set of\n"
      << "filters and producers configured for this job.  This error can\n"
      << "happen if a 'consumes' statement in one of the modules specifies\n"
      << "either the current process name or the literal string "
         "\"current_process\"\n"
      << "for the input tag.  If you have encountered this error under a\n"
      << "different circumstance, please contact artists@fnal.gov for "
         "guidance.\n";
  }
  return std::distance(begin_, it);
}

auto
art::detail::ModuleGraphInfoMap::info(std::string const& name) const
  -> ModuleGraphInfo const&
{
  return modules_[vertex_index(name)].second;
}
