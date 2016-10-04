#include "art/Framework/Art/detail/MetadataRegexHelpers.h"

using namespace std::string_literals;

std::regex
art::detail::regex_for_spec(std::string const& spec)
{
  return std::regex{"("+spec+"\\s*:\\s*)(:?<string>)"};
}

void
art::detail::replace_type(std::string& str, std::string const& spec, std::regex const& r)
{
  str = std::regex_replace(str, r, "$1"s + spec);
}
