#include "art/Framework/Core/detail/OutputModuleUtils.h"

#include <algorithm>

void
art::detail::
remove_whitespace(std::string & s)
{
  s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
  s.erase(std::remove(s.begin(), s.end(), '\t'), s.end());
}

void
art::detail::
parse_path_spec(std::string const & path_spec,
                parsed_path_spec_t & output)
{
  std::string trimmed_path_spec(path_spec);
  detail::remove_whitespace(trimmed_path_spec);
  std::string::size_type colon = trimmed_path_spec.find(":");
  if (colon == std::string::npos) {
    output.first = trimmed_path_spec;
  }
  else {
    output.first  = trimmed_path_spec.substr(0, colon);
    output.second =
      trimmed_path_spec.substr(colon + 1, trimmed_path_spec.size());
  }
}

