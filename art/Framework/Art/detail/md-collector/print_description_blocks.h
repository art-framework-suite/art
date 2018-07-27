#ifndef art_Framework_Art_detail_md_collector_print_description_blocks_h
#define art_Framework_Art_detail_md_collector_print_description_blocks_h

#include <string>

namespace art {
  namespace detail {
    class LibraryInfo;

    std::string print_header(LibraryInfo const& li,
                             std::string const& type_spec);

    std::string print_allowed_configuration(LibraryInfo const& li,
                                            std::string const& prefix,
                                            std::string const& type_spec);
  }
}

#endif /* art_Framework_Art_detail_md_collector_print_description_blocks_h */

// Local Variables:
// mode: c++
// End:
