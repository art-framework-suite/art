#include "art/Framework/Core/RootDictionaryManager.h"

#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>

#include "boost/regex.hpp"

#include "cetlib/exception.h"
#include "cpp0x/functional"
#include "Cintex/Cintex.h"

using std::placeholders::_1;

art::RootDictionaryManager::RootDictionaryManager()
   :
   dm_("dict"),
   mm_("map")
{
   // Enable Cintex so that dictionary information is jammed into CINT
   // when a Reflex dictionary is loaded.
   ROOT::Cintex::Cintex::Enable();

   // Load all dictionaries.
   dm_.loadAllLibraries();
   
}

std::ostream &art::RootDictionaryManager::
dumpReflexDictionaryInfo(std::ostream &os) const {
   lib_list_t loadedLibraries;
   dm_.getLoadedLibraries(loadedLibraries);
   lib_list_t::const_iterator i = loadedLibraries.begin(),
      end_iter = loadedLibraries.end();
   for (; i != end_iter; ++i) {
      dumpReflexDictionaryInfo(os, *i);
   }
}

std::ostream &art::RootDictionaryManager::
dumpReflexDictionaryInfo(std::ostream &os, std::string const &libpath) const {
   typedef void (*CapFunc)(const char **&, int &);
   std::ostringstream map_lib;
   std::ostream_iterator<char, char> oi(map_lib);
   boost::regex_replace(oi, libpath.begin(), libpath.end(),
                        boost::regex(std::string("_(") + dm_.libType() + ")" + dm_.dllExt() + "$"),
                        std::string("(?1" + mm_.libType() + ")"),
                        boost::match_default);
   CapFunc func = (CapFunc) mm_.getSymbolByPath(map_lib.str(), "SEAL_CAPABILITIES");
   if (func == nullptr) {
      // Throw correct exception.
      throw cet::exception("Unable to find properly constructed map library corresponding to dictionary" + libpath);
   }
   int size;
   char const ** names;
   func(names, size);
   for (int i=0; i<size; ++i) {
      os << std::setw(45) << libpath
         << "   "
         << std::setw(25) << names[i]
         << "\n";
   }
   return os;
}

bool art::RootDictionaryManager::
dictIsLoadable(std::string const &path) const {
   return dm_.libraryIsLoadable(path);
}

bool art::RootDictionaryManager::
dictIsLoaded(std::string const &path) const {
   return dm_.libraryIsLoaded(path);
}

void art::RootDictionaryManager::
loadAllDictionaries() {
   dm_.loadAllLibraries();
}
