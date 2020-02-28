#ifndef art_Framework_Art_detail_output_to_h
#define art_Framework_Art_detail_output_to_h

#include <string>

namespace art::detail {
  // The following functions allow a user to specify case-insensitive
  // strings such as "STDOUT", "cout", "STDERR", or "CERR" to indicate
  // that sending to an output to STDOUT or STDERR is desired.
  bool output_to_stderr(std::string const& spec);
  bool output_to_stdout(std::string const& spec);
}

#endif /* art_Framework_Art_detail_output_to_h */

// Local Variables:
// mode: c++
// End:
