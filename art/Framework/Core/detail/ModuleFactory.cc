#include "art/Framework/Core/detail/ModuleFactory.h"

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Utilities/PluginSuffixes.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/detail/wrapLibraryManagerException.h"

art::ModuleType
art::detail::ModuleFactory::moduleType(std::string const& libspec)
{
  ModuleTypeFunc_t* symbol = nullptr;
  try {
    lm_.getSymbolByLibspec(libspec, "moduleType", symbol);
  }
  catch (art::Exception& e) {
    cet::detail::wrapLibraryManagerException(e, "Module", libspec, getReleaseVersion());
  }
  if (symbol == nullptr) {
    throw art::Exception(errors::Configuration, "BadPluginLibrary")
      << "Module " << libspec
      << " with version " << getReleaseVersion()
      << " has internal symbol definition problems: consult an expert.";
  }
  return symbol();
}

std::unique_ptr<art::Worker>
art::detail::ModuleFactory::makeWorker(WorkerParams const& p, ModuleDescription const& md)
{
  std::string libspec(p.pset_.get<std::string>("module_type"));
  WorkerMaker_t* symbol = nullptr;
  try {
    lm_.getSymbolByLibspec(libspec, "make_worker", symbol);
  }
  catch (art::Exception& e) {
    cet::detail::wrapLibraryManagerException(e, "Module", libspec, getReleaseVersion());
  }
  if (symbol == nullptr) {
    throw art::Exception(errors::Configuration, "BadPluginLibrary: ")
      << "Module " << libspec
      << " with version " << getReleaseVersion()
      << " has internal symbol definition problems: consult an expert.";
  }
  return std::unique_ptr<Worker>(symbol(p, md));
}  // makeWorker()

// ======================================================================
