#include "art/Utilities/FirstAbsoluteOrLookupWithDotPolicy.h"
#include "cetlib/filesystem.h"

#include <cstdlib>

art::FirstAbsoluteOrLookupWithDotPolicy::
FirstAbsoluteOrLookupWithDotPolicy(std::string const &paths)
   :
   first(true),
   first_paths(std::string("./:") + paths),
   after_paths(paths)
{
}

std::string art::FirstAbsoluteOrLookupWithDotPolicy::operator() (std::string const &filename) {
   if (first) {
      first = false;
      return cet::is_absolute_filepath(filename)?filename:first_paths.find_file(filename);
   } else {
      return after_paths.find_file(filename);
   }
}

void art::FirstAbsoluteOrLookupWithDotPolicy::reset() {
   first = true;
}

art::FirstAbsoluteOrLookupWithDotPolicy::~FirstAbsoluteOrLookupWithDotPolicy() {
}
