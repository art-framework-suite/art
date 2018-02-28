#include "art/Framework/Core/detail/ModuleToPath.h"
#include "canvas/Utilities/Exception.h"

#include <algorithm>

using art::detail::module_name_t;
using art::detail::names_t;
using art::detail::modules_to_paths_t;
using art::detail::paths_to_modules_t;

namespace {
  auto map_to_vec(paths_to_modules_t const& paths)
  {
    std::map<module_name_t, names_t> tmp;
    for (auto const& path : paths) {
      for (auto const& mod : path.second) {
        tmp[mod].push_back(path.first);
      }
    }
    return modules_to_paths_t(cbegin(tmp), cend(tmp));
  }
}

art::detail::ModuleToPath::ModuleToPath(paths_to_modules_t const& paths)
  : modules_{map_to_vec(paths)}
  , begin_{cbegin(modules_)}
  , end_{cend(modules_)}
{}

auto
art::detail::ModuleToPath::find_vertex_index(std::string const& name) const -> distance_t
{
  auto const it = std::find_if(begin_, end_,
                               [&name](auto const& pr) { return pr.first == name; });
  if (it == end_) {
    throw art::Exception{art::errors::LogicError}
    << "The module name '"<< name << "' does not correspond to an element\n"
    << "in the set of filters and producers used to create the workflow graph.\n";
  }
  return std::distance(begin_, it);
}
