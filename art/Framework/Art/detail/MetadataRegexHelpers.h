#ifndef art_Framework_Art_detail_MetadataRegexHelpers_h
#define art_Framework_Art_detail_MetadataRegexHelpers_h

#include <string>

namespace art {
  namespace detail {

    void replace_module_type( std::string & str, std::string const& spec);
    void replace_label( std::string const & label, std::string & str );

  }
}

#endif

// Local variables:
// mode: c++
// End:
