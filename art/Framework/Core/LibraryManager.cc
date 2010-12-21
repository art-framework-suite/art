#include "art/Framework/Core/LibraryManager.h"

#include "boost/filesystem.hpp"
#include "cetlib/exception.h"
#include "cetlib/search_path.h"
#include "cpp0x/functional"
#include "boost/regex.hpp"

extern "C" {
#include <dlfcn.h>
}

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <vector>

using std::placeholders::_1;

// TODO: Should give "dylib" for Mac OS X.
std::string const art::LibraryManager::dll_ext_(".so");
std::string const art::LibraryManager::dll_ext_pat_("\\.so");

art::LibraryManager::LibraryManager(std::string const& lib_type)
   :
   lib_type_(lib_type),
   lib_loc_map_(),
   spec_trans_map_(),
   good_spec_trans_map_(),
   lib_ptr_map_()
{
   // TODO: We could also consider searching the ld.so.conf list, if
   // anyone asks for it.

   static cet::search_path const ld_lib_path("LD_LIBRARY_PATH");
   static std::string const pattern("lib[A-Za-z0-9_]*_");
   std::vector<std::string> matches;
   size_t n_matches = ld_lib_path.find_files(pattern + lib_type + dll_ext_pat_,
                                             matches);
   // Note the use of reverse iterators here: files found earlier in the
   // vector will therefore overwrite those found later, which is what
   // we want from "search path"-type behavior.
   std::for_each(matches.rbegin(), matches.rend(),
                 std::bind(&art::LibraryManager::lib_loc_map_inserter,
                           this,
                           _1));

   // Build the spec to long library name translation table.
   std::for_each(lib_loc_map_.begin(), lib_loc_map_.end(),
                 std::bind(&art::LibraryManager::spec_trans_map_inserter,
                           this,
                           _1,
                           std::ref(lib_type)));

   // Build the fast good-translation table.
   std::for_each(spec_trans_map_.begin(), spec_trans_map_.end(),
                 std::bind(&art::LibraryManager::good_spec_trans_map_inserter,
                           this,
                           _1));
}

void *art::LibraryManager::getSymbolByLibspec(std::string const &libspec,
                                              std::string const &sym_name)
   const
{
   std::string lib_name_str;

   good_spec_trans_map_t::const_iterator trans =
      good_spec_trans_map_.find(libspec);
   if (trans == good_spec_trans_map_.end()) {
      // No good translation => zero or too many
      std::ostringstream error_msg;
      error_msg
         << "Library specificaton \""
         << libspec << "\":";
      spec_trans_map_t::const_iterator bad_trans =
         spec_trans_map_.find(libspec);
      if (bad_trans != spec_trans_map_.end()) {
         error_msg << " corresponds to multiple libraries:\n";
         std::copy(bad_trans->second.begin(),
                   bad_trans->second.end(),
                   std::ostream_iterator<std::string>(error_msg, "\n"));
      } else {
         error_msg << " does not correspond to any library in LD_LIBRARY_PATH of type \""
                   << lib_type_
                   << "\"\n";
      }
      // TODO: Throw correct exception.
      throw cet::exception(error_msg.str());
   }
   return getSymbolByPath(trans->second, sym_name);
}

void *art::LibraryManager::getSymbolByPath(std::string const &lib_loc,
                                           std::string const &sym_name) const {
   void *result = nullptr;
   void *lib_ptr = get_lib_ptr(lib_loc);
   if (lib_ptr == nullptr) {
      // TODO: Throw correct exception.
      throw cet::exception("Unable to load requested library " + lib_loc);
   } else { // Found library
      dlerror();
      result = dlsym(lib_ptr, sym_name.c_str());
      char const *error = dlerror();
      if (error != nullptr) { // Error message
         result = nullptr;
         // TODO: Throw correct exception.
         throw cet::exception("Unable to load requested symbol " +
                              sym_name +
                              " from library " + 
                              lib_loc);
      }
   }
   return result;
}

size_t
art::LibraryManager::getLoadableLibraries(std::vector<std::string> &list)
   const
{
   return this->getLoadableLibraries(std::back_inserter(list));
}

size_t
art::LibraryManager::getLoadedLibraries(std::vector<std::string> &list) const
{
   return this->getLoadedLibraries(std::back_inserter(list));
}

size_t
art::LibraryManager::getValidLibspecs(std::vector<std::string> &list) const
{
   return this->getValidLibspecs(std::back_inserter(list));
}

void
art::LibraryManager::loadAllLibraries() const {
   lib_loc_map_t::const_iterator
      i = lib_loc_map_.begin(),
      end_iter = lib_loc_map_.end();
   for (;
        i != end_iter;
        ++i) {
      get_lib_ptr(i->second);
   }
}

bool
art::LibraryManager::libraryIsLoaded(std::string const & path) const
{
   return (lib_ptr_map_.find(path) != lib_ptr_map_.end());
}

bool
art::LibraryManager::libraryIsLoadable(std::string const & path) const
{
   // TODO: If called with any frequency, this should be made more
   // efficient.
   lib_loc_map_t::const_iterator
      i = lib_loc_map_.begin(),
      end_iter = lib_loc_map_.end();
   for (;
        i != end_iter;
        ++i) {
      if (path == i->second) { return true; }
   }
   return false;
}

void
art::LibraryManager::
lib_loc_map_inserter(std::string const &path) {
   lib_loc_map_[boost::filesystem::path(path).filename()] = path;
}

void 
art::LibraryManager::
spec_trans_map_inserter(lib_loc_map_t::value_type const& entry,
                        std::string const &lib_type) {
   // First obtain short spec.
   boost::regex e("([^_]+)_" + lib_type + dll_ext_pat_ + "$");
   boost::match_results<std::string::const_iterator> match_results;

   if (boost::regex_search(entry.first, match_results, e)) {
      spec_trans_map_[match_results[1]].insert(entry.second);
   } else {
      // TODO: Throw correct exception.
      throw cet::exception("Internal error in spec_trans_map_inserter for entry " + entry.first + " with pattern " + e.str());
   }

   // Next, convert library filename to full libspec.
   std::ostringstream lib_name;
   std::ostream_iterator<char, char> oi(lib_name);
   boost::regex_replace(oi, entry.first.begin(),
                        entry.first.end(),
                        boost::regex("(_+)"),
                        std::string("(?1/)"),
                        boost::match_default | boost::format_all);
   boost::regex stripper("^lib(.*)/" + lib_type + "\\..*$");
   std::string lib_name_str = lib_name.str();
   if (boost::regex_search(lib_name_str, match_results, stripper)) {
      spec_trans_map_[match_results[1]].insert(entry.second);
   } else {
      // TODO: Throw correct exception.
      throw cet::exception("Internal error in spec_trans_map_inserter stripping " +
                           lib_name.str());
   }
}

void
art::LibraryManager::
good_spec_trans_map_inserter(spec_trans_map_t::value_type const &entry) {
   if (entry.second.size() == 1) {
      good_spec_trans_map_[entry.first] = *(entry.second.begin());
   }
}

void *art::LibraryManager::get_lib_ptr(std::string const&lib_loc) const {
   void *lib_ptr = lib_ptr_map_[lib_loc];
   if (lib_ptr == nullptr) {
      lib_ptr_map_[lib_loc] = // Update cached ptr.
         lib_ptr = dlopen(lib_loc.c_str(), RTLD_LAZY | RTLD_LOCAL);
   }
   return lib_ptr;
}
