#include "art/Framework/Core/RootDictionaryManager.h"

#include "Cintex/Cintex.h"
#include "cetlib/exception.h"
#include "cpp0x/regex"
#include <iostream>
#include <iterator>
#include <sstream>

art::RootDictionaryManager::RootDictionaryManager()
  :
  dm_("dict"),
  mm_("map")
{
  // Enable Cintex so that dictionary information is jammed into CINT
  // when a Reflex dictionary is loaded.
  ROOT::Cintex::Cintex::Enable();
  // Load all dictionaries.
  loadAllDictionaries();
}

std::ostream & art::RootDictionaryManager::
dumpReflexDictionaryInfo(std::ostream & os) const
{
  lib_list_t loadedLibraries;
  dm_.getLoadedLibraries(loadedLibraries);
  lib_list_t::const_iterator i = loadedLibraries.begin(),
                             end_iter = loadedLibraries.end();
  for (; i != end_iter; ++i) {
    dumpReflexDictionaryInfo(os, *i);
  }
  return os;
}

std::ostream & art::RootDictionaryManager::
dumpReflexDictionaryInfo(std::ostream & os, std::string const & libpath) const
{
  typedef void (*CapFunc)(const char ** &, int &);
  std::ostringstream map_lib;
  std::ostream_iterator<char, char> oi(map_lib);
  std::regex_replace(oi, libpath.begin(), libpath.end(),
                     std::regex(std::string("_(") + dm_.libType() + ")(" + dm_.dllExt() + ")$"),
                     std::string("(?1_" + mm_.libType() + "$2)"),
                     boost::match_default | boost::format_all);
  CapFunc func = (CapFunc) mm_.getSymbolByPath(map_lib.str(), "SEAL_CAPABILITIES");
  if (func == nullptr) {
    // TODO: Throw correct exception.
    throw cet::exception("Unable to find properly constructed map library corresponding to dictionary" + libpath);
  }
  int size;
  char const ** names;
  func(names, size);
  for (int i = 0; i < size; ++i) {
    os << libpath
       << "   "
       << names[i]
       << "\n";
  }
  return os;
}

bool art::RootDictionaryManager::
dictIsLoadable(std::string const & path) const
{
  return dm_.libraryIsLoadable(path);
}

bool art::RootDictionaryManager::
dictIsLoaded(std::string const & path) const
{
  return dm_.libraryIsLoaded(path);
}

void art::RootDictionaryManager::
loadAllDictionaries()
{
  dm_.loadAllLibraries();
}
