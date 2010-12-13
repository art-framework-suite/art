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
      void* getSymbol(std::string libspec, std::string const& sym_name);

   private:

      typedef std::map<std::string, size_t> lib_loc_map_t;
      typedef std::map<std::string, std::set<std::string> > short_spec_trans_map_t;
      typedef std::map<std::string, void *> lib_ptr_map_t;
      typedef std::map<std::string, std::string> good_spec_trans_map_t;

      void lib_loc_map_inserter(lib_loc_map_t::key_type const &key,
                                lib_loc_map_t::mapped_type const &val);
      void short_spec_trans_map_inserter(lib_loc_map_t::value_type const &entry,
                                         std::string const &lib_type);
      void good_spec_trans_map_inserter(short_spec_trans_map_t::value_type const &entry);
      void *getSymbol_(std::string const &lib_loc, std::string const &sym_name);

      static std::string const dll_ext_;
      static std::string const dll_ext_pat_;
      std::string lib_type_;
      std::vector<std::string> ld_library_path_;
      lib_loc_map_t lib_loc_map_;
      short_spec_trans_map_t short_spec_trans_map_;
      good_spec_trans_map_t good_spec_trans_map_;
      lib_ptr_map_t lib_ptr_map_;
   };
}

#endif

// Local Variables:
// mode: c++
// End:
