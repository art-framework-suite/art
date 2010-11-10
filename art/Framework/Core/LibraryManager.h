#ifndef ART_FRAMEWORK_CORE_LIBRARY_MANAGER_H
#define ART_FRAMEWORK_CORE_LIBRARY_MANAGER_H

namespace art
{
  class LibraryManager
  {
  public:
    // Create a LibraryManager that searches through LD_LIBRARY_PATH
    // for dynamically loadable libraries having the given suffix.
    // If LD_LIBRARY_PATH is not defined, then no libraries are found.
    // Library names are expected to be of the form:
    //      libaa_bb_cc_xyz_<suffix>.<ext>
    //  and where <ext> is provided automatically as appropriate for
    // the platform.
    explicit LibraryManager(std::string const& suffix);

    // The d'tor does NOT unload libraries, because that is dangerous
    // to do in C++.
    ~LibraryManager();

    // Find and return a symbol named 'symname' in the library
    // identified by 'libspec'. The library is dynamically loaded if
    // necessary. If no library matching libspec is found, or if no
    // symbol named symname is found in the library, return a null
    // pointer. If more than one library matching libspec if found, an
    // exception is thrown.
    void* getSymbol(std::string const& libspec, std::string const& symname);

  private:
    
  };
}

#endif

// Local Variables:
// mode: c++
// End:
