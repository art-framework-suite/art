#include "art/Framework/Core/LibraryManager.h"

#include "cetlib/find_matching_files.h"
#include "cetlib/split.h"
#include "cetlib/split_path.h"
#include "cpp0x/functional"
#include "boost/regex.hpp"

extern "C" {
#include <dlfcn.h>
}

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <ostream>
#include <sstream>
#include <vector>

using std::placeholders::_1;

// TODO: Should give "dylib" for Mac OS X.
std::string const art::LibraryManager::dll_ext_(".so");
std::string const art::LibraryManager::dll_ext_pat_("\\.so");

void
art::LibraryManager::
good_spec_trans_map_inserter(short_spec_trans_map_t::value_type const &entry) {
   if (entry.second.size() == 1) {
      good_spec_trans_map_[entry.first] = *(entry.second.begin());
   }
}

void
art::LibraryManager::
lib_loc_map_inserter(lib_loc_map_t::key_type const &key,
                     lib_loc_map_t::mapped_type const &val) {
   lib_loc_map_[key] = val;
}

void 
art::LibraryManager::
short_spec_trans_map_inserter(lib_loc_map_t::value_type const& entry,
                              std::string const &lib_type) {
   static boost::regex e("([^_]+)_" + lib_type + "\\..*$");
   boost::match_results<std::string::const_iterator> match_results;

   if (boost::regex_search(entry.first, match_results, e)) {
      short_spec_trans_map_[match_results[0]].insert(entry.first);
   } else {
      // TODO: Throw!
   }
}

art::LibraryManager::LibraryManager(std::string const& lib_type)
   :
   lib_type_(lib_type)
{
  // TODO: We could also consider searching the ld.so.conf list, if
  // anyone asks for it.

  // If LD_LIBRARY_PATH is undefined, we have no libraries to find.
  const char* ld_library_path = getenv("LD_LIBRARY_PATH");
  if (ld_library_path == 0) return;

  const char* ld_library_path_end = ld_library_path + strlen(ld_library_path);
  cet::split(std::string(ld_library_path), ':', ld_library_path_.begin());

  // Go through the search path in reverse order so we overwrite with
  // the best place to find each library.
  std::vector<std::string>::const_reverse_iterator
     d_end  = ld_library_path_.rend(),
     i =  ld_library_path_.rbegin();
  size_t d_index = 0;
  for(;
      i != d_end;
      ++i, ++d_index) {
   std::vector<std::string> matches;
   std::string pattern("lib.*_");
   pattern += lib_type + dll_ext_pat_;

   // Get the list of wanted libraries in this directory
   cet::find_matching_files(pattern, *i, matches);
   std::for_each(matches.begin(),
                 matches.end(),
                 std::bind(&art::LibraryManager::lib_loc_map_inserter,
                           this,
                           _1,
                           std::ref(d_index)));
  }

  // Build the short-spec to long library name translation table.
  std::for_each(lib_loc_map_.begin(),
                lib_loc_map_.end(),
                std::bind(&art::LibraryManager::short_spec_trans_map_inserter,
                          this,
                          _1,
                          std::ref(lib_type)));
 
  // Build the fast good-translation table.
  std::for_each(short_spec_trans_map_.begin(),
                short_spec_trans_map_.end(),
                std::bind(&art::LibraryManager::good_spec_trans_map_inserter,
                          this,
                          _1));
}

void *art::LibraryManager::getSymbol(std::string  libspec,
                                     std::string const& sym_name) {
   std::string lib_name_str;
   lib_loc_map_t::const_iterator loc;

   if (libspec.find('/') == std::string::npos) {
      // Short specification
      good_spec_trans_map_t::const_iterator trans =
         good_spec_trans_map_.find(libspec);
      if (trans != good_spec_trans_map_.end()) {
         libspec = trans->second; // Good translation
         loc = lib_loc_map_.find(libspec);
         if (loc == lib_loc_map_.end()) {
            // Internal Error!
            // TODO: Throw!
         }
      } else { // Bad translation or not found
         std::ostringstream error_msg;
         error_msg <<
            "Short specificaton \""
                   << libspec << "\":";
         short_spec_trans_map_t::const_iterator bad_trans =
            short_spec_trans_map_.find(libspec);
         if (bad_trans != short_spec_trans_map_.end()) {
            error_msg << " corresponds to multiple libraries:\n";
            std::copy(bad_trans->second.begin(),
                      bad_trans->second.end(),
                      std::ostream_iterator<std::string>(error_msg, "\n"));
         } else {
            error_msg << " does not correspond to any library in LD_LIBRARY_PATH of type \""
                      << lib_type_
                      << "\"\n";
         }
         // TODO: Throw!
      }
   } else { // Have long-format specification
      loc = lib_loc_map_.find(libspec);
      if (loc != lib_loc_map_.end()) {
         std::ostringstream lib_name;
         lib_name << "lib";
         std::ostream_iterator<char, char> oi(lib_name);
         boost::regex_replace(oi, libspec.begin(),
                              libspec.end(),
                              boost::regex("(_+)"),
                              std::string("(?1/)"),
                              boost::match_default | boost::format_all);
         lib_name << lib_type_ << dll_ext_;
         lib_name_str = lib_name.str();
      } else {
         // TODO: Throw!
      }
   }
   return getSymbol_(ld_library_path_[loc->second] + '/' +
                     lib_name_str,
                     sym_name);
   
}

void *art::LibraryManager::getSymbol_(std::string const &lib_loc,
                                      std::string const &sym_name) {
   void *result = nullptr;
   void *&lib_ptr = lib_ptr_map_[lib_loc];
   if (lib_ptr == nullptr) {
      lib_ptr = dlopen(lib_loc.c_str(),
                       RTLD_LAZY | RTLD_LOCAL);
   }
   if (lib_ptr == nullptr) {
      // TODO: Log message indicating library load error.
   } else { // Found library
      dlerror();
      result = dlsym(lib_ptr, sym_name.c_str());
      std::string sym_error(dlerror());
      if (!sym_error.empty()) { // Error message
         result = nullptr;
         // TODO: Log message indicating symbol error.
      }
   }
   return result;
}
