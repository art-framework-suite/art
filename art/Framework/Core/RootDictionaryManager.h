#ifndef art_Framework_CoreRootDictionaryManager_h
#define art_Framework_CoreRootDictionaryManager_h

#include "art/Framework/Core/LibraryManager.h"

#include <iosfwd>
#include <string>

namespace art {
   class RootDictionaryManager {
   public:
      RootDictionaryManager();

      std::ostream & dumpReflexDictionaryInfo(std::ostream &os) const;

      std::ostream & dumpReflexDictionaryInfo(std::ostream &os,
                                              std::string const &libpath) const;

      bool dictIsLoadable(std::string const &path) const;
      bool dictIsLoaded(std::string const &path) const;

   private:
      typedef std::vector<std::string> lib_list_t;
      void loadAllDictionaries();

      LibraryManager dm_; // Dictionaries
      LibraryManager mm_; // Maps
   };
}

#endif // art_Framework_Core_RootDictionaryManager_h
