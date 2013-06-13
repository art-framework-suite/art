#ifndef art_Framework_Core_detail_ModuleFactory_h
#define art_Framework_Core_detail_ModuleFactory_h

////////////////////////////////////////////////////////////////////////
// ModuleFactory
//
// Manage the creation of workers.
//
////////////////////////////////////////////////////////////////////////
#include "art/Utilities/LibraryManager.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Persistency/Provenance/ModuleDescription.h"

#include <memory>
#include <string>

namespace art {
  namespace detail {
    class ModuleFactory;
  }
}

class art::detail::ModuleFactory {
public:
  ModuleFactory();

  ModuleFactory(ModuleFactory const &) = delete;
  ModuleFactory & operator = (ModuleFactory const &) = delete;


  std::unique_ptr<Worker>
  makeWorker(WorkerParams const & wp,
             ModuleDescription const & md);

private:
  LibraryManager lm_;

};
#endif /* art_Framework_Core_detail_ModuleFactory_h */

// Local Variables:
// mode: c++
// End:
