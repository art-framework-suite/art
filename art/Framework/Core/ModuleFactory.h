#ifndef FWCore_Framework_ModuleFactory_h
#define FWCore_Framework_ModuleFactory_h

// ======================================================================
//
// ModuleFactory
//
// ======================================================================

#include "art/Framework/Core/LibraryManager.h"
#include "art/Framework/Core/Worker.h"
#include "art/Framework/Core/WorkerParams.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "cpp0x/memory"
#include <map>
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
    makeWorker( std::string       const & kind
              , WorkerParams      const & wp
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

#endif
