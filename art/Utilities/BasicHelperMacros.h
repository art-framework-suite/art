#ifndef art_Utilities_BasicHelperMacros_h
#define art_Utilities_BasicHelperMacros_h

////////////////////////////////////////////////////////////////////////
//
// HelperMacros
//
////////////////////////////////////////////////////////////////////////

#include <string>
#include "boost/filesystem.hpp"

namespace bfs = boost::filesystem;

#define PROVIDE_FILE_PATH()                              \
  extern "C"                                             \
  std::string                                            \
  get_source_location()                                  \
  {                                                      \
    bfs::path const p( __FILE__ );                       \
    return bfs::complete(p).native();                    \
  }

#endif /* art_Utilities_BasicHelperMacros_h */

// Local Variables:
// mode: c++
// End:
