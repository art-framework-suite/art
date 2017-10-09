#include "art/Framework/Art/detail/fillSourceList.h"

void
art::detail::fillSourceList(std::istream& is,
                            std::vector<std::string>& source_list)
{
  for (std::string line; std::getline(is, line);) {
    auto const comment_start = line.find('#');
    if (comment_start != std::string::npos)
      line.erase(comment_start);
    if (!line.empty())
      source_list.push_back(line);
  }
}
