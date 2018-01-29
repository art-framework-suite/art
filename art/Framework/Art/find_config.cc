// ======================================================================
//
// find_config
//
// ======================================================================

#include "art/Framework/Art/find_config.h"

#include "cetlib/search_path.h"
#include "cetlib_except/exception.h"

bool
art::find_config(std::string const& filename,
                 std::string const& search_path_spec,
                 std::string& full_path)
{
  try {
    cet::search_path sp(search_path_spec);
    if (!sp.find_file(filename, full_path)) {
      return false;
    }
  }
  catch (cet::exception const& e) {
    if (e.root_cause() == "getenv") {
      // Assume file is findable as specified.
      full_path = filename;
    } else {
      throw;
    }
  }
  return true;
}
