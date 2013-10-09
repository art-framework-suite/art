#ifndef art_Framework_Core_detail_OutputModuleUtils_h
#define art_Framework_Core_detail_OutputModuleUtils_h

#include <string>
#include <utility>

namespace art {
namespace detail {
void remove_whitespace(std::string&);
void parse_path_spec(std::string const& path_spec,
                     std::pair<std::string, std::string>& output);
}
}

#endif // art_Framework_Core_detail_OutputModuleUtils_h
