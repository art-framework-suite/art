// ======================================================================
//
// ModuleFactory
//
// ======================================================================

#include "art/Framework/Core/ModuleFactory.h"

#include "art/Framework/Core/detail/wrapLibraryManagerException.h"
#include "art/Utilities/Exception.h"

using namespace art;

// ----------------------------------------------------------------------

ModuleFactory::ModuleFactory()
  : lm_("module")
{ }

ModuleFactory::~ModuleFactory()
{ }

// ----------------------------------------------------------------------

ModuleFactory &
ModuleFactory::the_factory_()
{
  static ModuleFactory the_factory;
  return the_factory;
}

// ----------------------------------------------------------------------

std::auto_ptr<Worker>
ModuleFactory::makeWorker(WorkerParams      const & p
                          , ModuleDescription const & md
                         )
{
  std::string libspec(p.pset_->get<std::string>("module_type"));
  typedef Worker* (make_t)(WorkerParams      const &
                           , ModuleDescription const &
                          );
  make_t * symbol = nullptr;
  try {
    // reinterpret_cast is required because void* can't be converted to a whole lot.
    symbol = reinterpret_cast<make_t *>(the_factory_().lm_.getSymbolByLibspec(libspec, "make_temp"));
  }
  catch (art::Exception & e) {
    detail::wrapLibraryManagerException(e, "Module", libspec, p.releaseVersion_);
  }
  if (symbol == nullptr) {
    throw art::Exception(errors::Configuration, "BadPluginLibrary")
        << "Module " << libspec
        << " with version " << p.releaseVersion_
        << " has internal symbol definition problems: consult an expert.";
  }
  return std::auto_ptr<Worker>(symbol(p, md));
}  // makeWorker()

// ======================================================================
