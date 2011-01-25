#include "art/Framework/Core/find_config.h"
#include "cetlib/exception.h"
#include "cetlib/search_path.h"

#include <cstdlib>
#include <iostream>

bool art::find_config(std::string const &filename,
                      std::string const &search_path_spec,
                      std::string &full_path) {
   try {
      cet::search_path sp(search_path_spec);
      if (!sp.find_file(filename, full_path)) {
         return false;
      }
   }
   catch (cet::exception const &e) {
      if (e.root_cause() == "getenv") {
         // Assume file is findable as specified.
         full_path = filename;
      } else {
         throw;
      }
   }
   return true;
}
