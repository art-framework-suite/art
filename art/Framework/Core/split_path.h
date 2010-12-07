#ifndef ART_FRAMEWORK_CORE_SPLIT_PATH_h
#define ART_FRAMEWORK_CORE_SPLIT_PATH_h

#include <string>
#include <vector>

namespace art
{
  // Split the string 'path' into components delimited by a single
  // colon. Adjacent colons result in an empty string.
  void split_path(std::string const& path, std::vector<std::string>& components);
}

#endif //  ART_FRAMEWORK_CORE_SPLIT_PATH_h

// Local Variables:
// mode: c++
// End:
