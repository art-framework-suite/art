#ifndef art_Framework_Core_ModuleFactory_h
#define art_Framework_Core_ModuleFactory_h

// ======================================================================
//
// ModuleFactory
//
// ======================================================================

#include "art/Utilities/LibraryManager.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "cpp0x/memory"
#include <string>

namespace art {
  class ModuleFactory;
}

// ----------------------------------------------------------------------

class art::ModuleFactory
{
  // non-copyable:
  ModuleFactory( ModuleFactory const & );
  void  operator = ( ModuleFactory const & );

public:
  static std::auto_ptr<Worker>
    makeWorker( WorkerParams      const & wp
              , ModuleDescription const & md
              );

private:
  LibraryManager lm_;

  ModuleFactory();
  ~ModuleFactory();

  static ModuleFactory &
    the_factory_();

};  // ModuleFactory

// ======================================================================

#endif /* art_Framework_Core_ModuleFactory_h */

// Local Variables:
// mode: c++
// End:
