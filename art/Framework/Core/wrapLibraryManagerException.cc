#include "art/Framework/Core/wrapLibraryManagerException.h"

void art::wrapLibraryManagerException(art::Exception const &e,
                                      std::string const &item_type,
                                      std::string const &libspec,
                                      std::string const &release) {
   if (e.categoryCode() == art::errors::LogicError) {
      // Re-throw.
      throw;
   } else {
      // Wrap and throw.
      throw art::Exception(errors::Configuration,
                           std::string("Unknown") + item_type,
                           e)
         << item_type + " " << libspec
         << " with version " << release
         << " was not registered.\n"
         << "Perhaps your module type is misspelled or is not a framework plugin.";
   }
}
