#include "art/Framework/Core/detail/OutputModuleUtils.h"

#include <algorithm>
#include <string>
#include <utility>

using namespace std;

namespace art {
namespace detail {

void
remove_whitespace(string& S)
{
  S.erase(remove(S.begin(), S.end(), ' '), S.end());
  S.erase(remove(S.begin(), S.end(), '\t'), S.end());
}

void
parse_path_spec(string const& PS, pair<string,string>& PPS)
{
  string T(PS);
  detail::remove_whitespace(T);
  string::size_type C = T.find(":");
  if (C == string::npos) {
    PPS.first = T;
    return;
  }
  PPS.first  = T.substr(0, C);
  PPS.second = T.substr(C + 1);
}

} // namespace art
} // namespace detail

