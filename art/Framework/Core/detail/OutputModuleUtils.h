#ifndef art_Framework_Core_detail_OutputModuleUtils_h
#define art_Framework_Core_detail_OutputModuleUtils_h

#include <string>
#include <utility>

namespace art {
  namespace detail {
    void remove_whitespace(std::string & s);

    typedef std::pair<std::string, std::string> parsed_path_spec_t;

    void parse_path_spec(std::string const & path_spec,
                         parsed_path_spec_t & output);
  }
}

#endif /* art_Framework_Core_detail_OutputModuleUtils_h */

// Local Variables:
// mode: c++
// End:
