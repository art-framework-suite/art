#ifndef art_Framework_Core_RootDictionaryManager_h
#define art_Framework_Core_RootDictionaryManager_h

// ======================================================================
//
// RootDictionaryManager
//
// ======================================================================

#include "art/Utilities/LibraryManager.h"
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

#endif /* art_Framework_Core_RootDictionaryManager_h */

// Local Variables:
// mode: c++
// End:
