#ifndef ART_FRAMEWORK_CORE_LIBRARY_MANAGER_H
#define ART_FRAMEWORK_CORE_LIBRARY_MANAGER_H

#include <string>
#include <map>
#include <set>
#include <vector>

namespace art
{
   class LibraryManager
   {
   public:
      // Create a LibraryManager that searches through LD_LIBRARY_PATH
      // for dynamically loadable libraries having the given lib_type.
      // If LD_LIBRARY_PATH is not defined, then no libraries are found.
      // Library names are expected to be of the form:
      //      libaa_bb_cc_xyz_<lib_type>.<ext>
      //  and where <ext> is provided automatically as appropriate for
      // the platform.
      explicit LibraryManager(std::string const& lib_type);

      // The d'tor does NOT unload libraries, because that is dangerous
      // to do in C++. Use the compiler-generated destructor.
      // ~LibraryManager();

      // Find and return a symbol named 'sym_name' in the library
      // identified by 'libspec'. The library is dynamically loaded if
      // necessary. If no library matching libspec is found, or if no
      // symbol named sym_name is found in the library, return a null
      // pointer. If more than one library matching libspec if found, an
      // exception is thrown.
      void* getSymbol(std::string libspec, std::string const& sym_name) const;

      // Get a list of loadable libraries (full paths). Returns the
      // number of entries.
      size_t getLoadableLibraries(std::vector<std::string> &list) const;
      template <class OutIter>
      size_t getLoadableLibraries(OutIter dest) const;

      // Get list of valid libspecs. Returns the number of entries.
      size_t getValidLibspecs(std::vector<std::string> &list) const;
      template <class OutIter>
      size_t getValidLibspecs(OutIter dest) const;

      // Load all libraries at once.
      void loadAllLibraries() const;

   private:
      // Internally-useful typedefs.
      typedef std::map<std::string, std::string> lib_loc_map_t;
      typedef std::map<std::string, std::set<std::string> > spec_trans_map_t;
      typedef std::map<std::string, void *> lib_ptr_map_t;
      typedef std::map<std::string, std::string> good_spec_trans_map_t;

      // Private helper functions.
      void lib_loc_map_inserter(std::string const &path);
      void spec_trans_map_inserter(lib_loc_map_t::value_type const &entry,
                                         std::string const &lib_type);
      void good_spec_trans_map_inserter(spec_trans_map_t::value_type const &entry);
      void *get_lib_ptr(std::string const &lib_loc) const;
      void *getSymbol_(std::string const &lib_loc,
                       std::string const &sym_name) const;

      static std::string const dll_ext_; // .so or .dylib
      static std::string const dll_ext_pat_; // \\.so or \\.dylib
      std::string lib_type_; // eg _plugin.
      // Map of library filename -> full path.
      lib_loc_map_t lib_loc_map_;
      // Map of spec -> full path.
      spec_trans_map_t spec_trans_map_;
      // Map of only good translations.
      good_spec_trans_map_t good_spec_trans_map_;
      // Cache of already-loaded libraries.
      mutable lib_ptr_map_t lib_ptr_map_;
   };
}

template <class OutIter>
size_t art::LibraryManager::getLoadableLibraries(OutIter dest) const {
   size_t count = 0;
   lib_loc_map_t::const_iterator
      i = lib_loc_map_.begin(),
      end_iter = lib_loc_map_.end();
   for (;
        i != end_iter;
        ++i, ++count) {
      *dest++ = i->second;
   }
   return count;
}

template <class OutIter>
size_t art::LibraryManager::getValidLibspecs(OutIter dest) const {
   size_t count = 0;
   spec_trans_map_t::const_iterator
      i = spec_trans_map_.begin(),
      end_iter = spec_trans_map_.end();
   for (;
        i != end_iter;
        ++i, ++count) {
      *dest++ = i->first;
   }
   return count;
}



#endif

// Local Variables:
// mode: c++
// End:
