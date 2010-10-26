#ifndef LibraryLoader_RootAutoLibraryLoader_h
#define LibraryLoader_RootAutoLibraryLoader_h

/**\class RootAutoLibraryLoader
 *
 * ROOT helper class which can automatically load the
 * proper shared library when ROOT needs a new class dictionary
 *
 */


#include "TClassGenerator.h"

class DummyClassToStopCompilerWarning;

namespace art {

  class RootAutoLibraryLoader
    : public TClassGenerator
  {
    friend class DummyClassToStopCompilerWarning;
  public:
    /// return class type
    virtual TClass *GetClass(const char* classname, Bool_t load);
    /// return class type
    virtual TClass *GetClass(const type_info& typeinfo, Bool_t load);
    /// interface for TClass generators
    //ClassDef(RootAutoLibraryLoader,1);
    /// enable automatic library loading
    static void enable();

    /// load all known libraries holding dictionaries
    static void loadAll();

  private:
    const char* classNameAttemptingToLoad_; //!
    RootAutoLibraryLoader();
    RootAutoLibraryLoader(const RootAutoLibraryLoader&); // stop default
    const RootAutoLibraryLoader& operator=(const RootAutoLibraryLoader&); // stop default
  };

}  // namespace art

#endif  // LibraryLoader_RootAutoLibraryLoader_h
