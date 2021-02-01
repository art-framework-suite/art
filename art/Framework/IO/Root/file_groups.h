#ifndef art_Framework_IO_Root_file_groups_h
#define art_Framework_IO_Root_file_groups_h

#include <iosfwd>
#include <string>
#include <tuple>
#include <vector>

namespace art {
  using entry_t = std::tuple<std::string, std::vector<std::string>>;
  using collection_t = std::vector<entry_t>;

  bool file_group(std::string line, entry_t& group);
  collection_t file_groups(std::istream& file_list);
}

#endif /* art_Framework_IO_Root_file_groups_h */

// Local Variables:
// mode: c++
// End:
