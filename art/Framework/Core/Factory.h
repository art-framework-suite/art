#ifndef FWCore_Framework_Factory_h
#define FWCore_Framework_Factory_h

#include "art/Framework/Core/Worker.h"
#include "art/Framework/Core/WorkerParams.h"
#include "art/Framework/Core/LibraryManager.h"

#include <map>
#include <string>
#include <memory>

namespace art {


  class Factory
  {
  public:
    // We are not caching the discovered symbols. If we did, we'd use
    // a map of typespec (string) to maker-function-pointer

    static
    std::auto_ptr<Worker> makeWorker(WorkerParams const wp&,
				     ModuleDescription const& md);

  private:
    static Factory& the_instance_();
    Factory();
    ~Factory();
    LibraryManager libman;    
  };

}
#endif
