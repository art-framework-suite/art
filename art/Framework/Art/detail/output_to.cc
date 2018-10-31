#include "art/Framework/Art/detail/output_to.h"

#include <regex>

namespace {
  std::regex const re_stdout{R"((STDOUT|cout))",
                             std::regex_constants::ECMAScript |
                               std::regex_constants::icase};
  std::regex const re_stderr{R"((STDERR|cerr))",
                             std::regex_constants::ECMAScript |
                               std::regex_constants::icase};
}

bool
art::detail::output_to_stderr(std::string const& spec)
{
  return std::regex_match(spec, re_stderr);
}

bool
art::detail::output_to_stdout(std::string const& spec)
{
  return std::regex_match(spec, re_stdout);
}
