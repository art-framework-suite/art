#include "art/Utilities/detail/remove_whitespace.h"

#include "boost/algorithm/string.hpp"
#include "boost/range/algorithm_ext.hpp"

namespace {
  std::string const whitespace_chars{" \t"};
}

void
art::detail::remove_whitespace(std::string& str)
{
  boost::remove_erase_if(str, boost::is_any_of(whitespace_chars));
}

bool
art::detail::has_whitespace(std::string const& str)
{
  return str.find_first_of(whitespace_chars) != std::string::npos;
}
