#include "art/Framework/Core/detail/parse_path_spec.h"

#include <algorithm>
#include <string>
#include <utility>

using namespace std;

void
art::detail::remove_whitespace(string& s)
{
  s.erase(remove(s.begin(), s.end(), ' '), s.end());
  s.erase(remove(s.begin(), s.end(), '\t'), s.end());
}

void
art::detail::parse_path_spec(string path, pair<string,string>& pname_path)
{
  detail::remove_whitespace(path);
  auto const pos = path.find(":");
  if (pos == string::npos) {
    pname_path = std::make_pair("", path);
    return;
  }
  pname_path = make_pair(path.substr(0, pos), path.substr(pos+1));
}
