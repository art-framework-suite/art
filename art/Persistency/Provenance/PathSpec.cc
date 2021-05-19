#include "art/Persistency/Provenance/PathSpec.h"
#include "art/Utilities/detail/remove_whitespace.h"
#include "cetlib/container_algorithms.h"

#include <ostream>
#include <string>
#include <utility>

namespace art {
  std::pair<std::string, std::string>
  split_process_and_path_names(std::string path_spec)
  {
    detail::remove_whitespace(path_spec);
    auto const pos = path_spec.find(":");
    if (pos == std::string::npos) {
      return std::make_pair("", path_spec);
    }
    return std::make_pair(path_spec.substr(0, pos), path_spec.substr(pos + 1));
  }

  PathSpec
  path_spec(std::string const& path_spec)
  {
    auto const colon_position = path_spec.find(":");
    if (colon_position == std::string::npos) {
      return PathSpec{path_spec, PathID::invalid()};
    }
    auto name = path_spec.substr(colon_position + 1);
    auto const id = std::stoull(path_spec.substr(0, colon_position));
    return PathSpec{move(name), PathID{id}};
  }

  std::vector<PathSpec>
  path_specs(std::vector<std::string> const& path_spec_strs)
  {
    std::vector<PathSpec> result;
    cet::for_all_with_index(path_spec_strs,
                            [&result](size_t const i, auto const& str) {
                              auto spec = path_spec(str);
                              if (spec.path_id == PathID::invalid()) {
                                spec.path_id = PathID{i};
                              }
                              result.push_back(std::move(spec));
                            });
    return result;
  }

  std::string
  to_string(PathID const id)
  {
    return std::to_string(id.id_);
  }

  std::string
  to_string(PathSpec const& spec)
  {
    return to_string(spec.path_id) + ':' + spec.name;
  }

  std::ostream&
  operator<<(std::ostream& os, PathSpec const& spec)
  {
    return os << to_string(spec);
  }
}
