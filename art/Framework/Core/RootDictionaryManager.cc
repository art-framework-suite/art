#include "art/Framework/Core/RootDictionaryManager.h"

#include "art/Utilities/Exception.h"
#include "cetlib/shlib_utils.h"

#include <iostream>
#include <iterator>
#include <regex>
#include <sstream>

#include "Cintex/Cintex.h"

art::RootDictionaryManager::RootDictionaryManager()
   :
  dm_("dict", "([-A-Za-z0-9]*_)*[-A-Za-z0-9]+_"),
  mm_("map", "([-A-Za-z0-9]*_)*[-A-Za-z0-9]+_")
{
   // Enable Cintex so that dictionary information is jammed into CINT
   // when a Reflex dictionary is loaded.
   ROOT::Cintex::Cintex::Enable();

   // Load all dictionaries.
   loadAllDictionaries();
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
  return os;
}

std::ostream &
art::RootDictionaryManager::
dumpReflexDictionaryInfo(std::ostream &os, std::string const &libpath) const {
   typedef void (*CapFunc)(const char **&, int &);
   std::ostringstream map_lib;
   std::ostream_iterator<char, char> oi(map_lib);
   std::regex_replace(oi, libpath.begin(), libpath.end(),
                      std::regex(std::string("_(") +
                                 dm_.libType() + ")(\\" +
                                 cet::shlib_suffix() +
                                 ")$"),
                      std::string("(?1_" + mm_.libType() + "$2)"),
                      boost::match_default | boost::format_all);
   CapFunc func = mm_.getSymbolByPath<CapFunc>(map_lib.str(), "SEAL_CAPABILITIES");
   if (func == nullptr) {
      // TODO: Throw correct exception.
     throw Exception(errors::DictionaryNotFound)
       << "Unable to find properly constructed map library corresponding to "
       << "dictionary "
       << libpath;
   }
   int size;
   char const ** names;
   func(names, size);
   for (int i=0; i<size; ++i) {
      os << libpath
         << "   "
         << names[i]
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
