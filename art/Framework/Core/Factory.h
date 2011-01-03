#ifndef FWCore_Framework_Factory_h
#define FWCore_Framework_Factory_h

// ======================================================================
//
// Factory
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
  class Factory;
}

// ----------------------------------------------------------------------

class art::Factory
{
  // non-copyable:
  Factory( Factory const & );
  void  operator = ( Factory const & );

public:
  static std::auto_ptr<Worker>
    makeWorker( std::string       const & kind
              , WorkerParams      const & wp
              , ModuleDescription const & md
              );

private:
  typedef  std::map< std::string
                   , std::shared_ptr<LibraryManager>
                   >
           mgr_map_t;
  mgr_map_t  mgr_map_;

  Factory();
  ~Factory();

  static Factory &
    the_factory_();

};  // Factory

// ======================================================================

#endif
