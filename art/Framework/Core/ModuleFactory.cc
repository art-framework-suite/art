// ======================================================================
//
// ModuleFactory
//
// ======================================================================

#include "art/Framework/Core/ModuleFactory.h"

#include "art/Utilities/Exception.h"
#include <utility>

using namespace art;

// ----------------------------------------------------------------------

ModuleFactory::ModuleFactory()
: lm_( "module" )
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
ModuleFactory::makeWorker( std::string       const & kind
                           , WorkerParams      const & p
                           , ModuleDescription const & md
                           )
{
   std::string libspec( p.pset_->get<std::string>("_module_type") );
   typedef Worker*  (*make_t)( WorkerParams      const &
                               , ModuleDescription const &
                               );
   make_t * symbol = nullptr;
   try {
      make_t * symbol
         = static_cast<make_t*>( the_factory_().lm_.getSymbolByLibspec(libspec, "make_temp") );
   }
   catch (cet::exception e) {
      throw art::Exception(errors::Configuration,"UnknownModule", e)
         << "Module " << libspec
         << " with version " << p.releaseVersion_
         << " was not registered.\n"
         << "Perhaps your module type is misspelled or is not a framework plugin.";
   }
   if (symbol == nullptr) {
      throw art::Exception(errors::Configuration, "BadPluginLibrary")
         << "Module " << libspec
         << " with version " << p.releaseVersion_
         << " has internal symbol definition problems: consult an expert.";
   }
   return std::auto_ptr<Worker>( (*symbol)(p,md) );

}  // makeWorker()

// ======================================================================
