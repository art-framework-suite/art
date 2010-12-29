#ifndef art_Framework_CoreRootDictionaryManager_h
#define art_Framework_CoreRootDictionaryManager_h

// ======================================================================
//
// RootDictionaryManager
//
// ======================================================================

#include "art/Framework/Core/LibraryManager.h"
#include <iosfwd>
#include <string>
#include <vector>

// ----------------------------------------------------------------------

namespace art {

  class RootDictionaryManager
  {
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
  };  // RootDictionaryManager

}  // art

// ======================================================================

#endif
