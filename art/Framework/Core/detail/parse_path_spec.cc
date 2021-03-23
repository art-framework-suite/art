#include "art/Framework/Core/detail/parse_path_spec.h"
#include "art/Framework/Core/detail/remove_whitespace.h"

#include <string>
#include <utility>

std::pair<std::string, std::string>
art::detail::parse_path_spec(std::string path)
{
  remove_whitespace(path);
  auto const pos = path.find(":");
  if (pos == std::string::npos) {
    return std::make_pair("", path);
  }
  return std::make_pair(path.substr(0, pos), path.substr(pos + 1));
}
