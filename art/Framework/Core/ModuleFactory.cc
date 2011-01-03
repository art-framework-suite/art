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
: mgr_map_( )
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
  std::shared_ptr<LibraryManager> libmgr_p;
  mgr_map_t & mm = ModuleFactory::the_factory_().mgr_map_;
  mgr_map_t::iterator it = mm.find(kind);
  if( it == mm.end() ) {
    libmgr_p.reset( new LibraryManager(kind) );
    mm.insert( std::make_pair(kind, libmgr_p) );
  }
  else
    libmgr_p = it->second;

  std::string libspec( p.pset_->get<std::string>("_module_type") );
  typedef Worker*  (*make_t)( WorkerParams      const &
                            , ModuleDescription const &
                            );
  make_t * symbol
    = static_cast<make_t*>( libmgr_p->getSymbolByLibspec(libspec, "make_temp") );
  if( symbol == 0 )
    throw art::Exception(errors::Configuration,"UnknownModule")
      << "Module " << libspec
      << " with version " << p.releaseVersion_
      << " was not registered.\n"
      << "Perhaps your module type is misspelled or is not a framework plugin.";

  return std::auto_ptr<Worker>( (*symbol)(p,md) );

}  // makeWorker()

// ======================================================================
