#ifndef art_Framework_Art_detail_fillSourceList_h
#define art_Framework_Art_detail_fillSourceList_h

#include <istream>
#include <string>
#include <vector>

namespace art {
  namespace detail {

    void fillSourceList(std::istream& ifs, std::vector<std::string> & source_list);

  }
}

#endif /* art_Framework_Art_detail_fillSourceList_h */

// Local variables:
// mode: c++
// End:
