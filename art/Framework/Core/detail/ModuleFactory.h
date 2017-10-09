#ifndef art_Framework_Core_detail_ModuleFactory_h
#define art_Framework_Core_detail_ModuleFactory_h

////////////////////////////////////////////////////////////////////////
// ModuleFactory
//
// Manage the creation of workers.
//
////////////////////////////////////////////////////////////////////////
#include "art/Framework/Core/ModuleType.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Utilities/PluginSuffixes.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "cetlib/LibraryManager.h"

#include <memory>
#include <string>

namespace art {
  namespace detail {
    class ModuleFactory;
  }
}

class art::detail::ModuleFactory {
public:
  ModuleFactory() = default;

  ModuleFactory(ModuleFactory const&) = delete;
  ModuleFactory& operator=(ModuleFactory const&) = delete;

  ModuleType moduleType(std::string const& libspec);

  std::unique_ptr<Worker> makeWorker(WorkerParams const& wp,
                                     ModuleDescription const& md);

private:
  cet::LibraryManager lm_{Suffixes::module()};
};
#endif /* art_Framework_Core_detail_ModuleFactory_h */

// Local Variables:
// mode: c++
// End:
