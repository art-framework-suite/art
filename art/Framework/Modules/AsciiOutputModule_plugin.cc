// ======================================================================
//
// AsciiOutputModule_plugin
//
// ======================================================================


#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/MakerMacros.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Persistency/Provenance/Provenance.h"

#include "fhiclcpp/ParameterSet.h"
  using fhicl::ParameterSet;
#include "MessageFacility/MessageLogger.h"

#include <ostream>  // endl
#include <vector>


// Contents:
namespace art {
  class AsciiOutputModule;
}
using art::AsciiOutputModule;


// ======================================================================


class art::AsciiOutputModule
  : public OutputModule
{
public:
  // We do not take ownership of passed stream.
  explicit AsciiOutputModule( ParameterSet const & );
  virtual ~AsciiOutputModule();

private:
  virtual void write      ( EventPrincipal const & );
  virtual void writeSubRun( SubRunPrincipal const & ) { }
  virtual void writeRun   ( RunPrincipal const & ) { }
  int prescale_;
  int verbosity_;
  int counter_;

};  // AsciiOutputModule


// ======================================================================


AsciiOutputModule::AsciiOutputModule(ParameterSet const & pset)
  : OutputModule( pset )
  , prescale_   ( pset.get<unsigned int>("prescale", 1U) )
  , verbosity_  ( pset.get<unsigned int>("verbosity", 1U) )
  , counter_    ( 0 )
{
  if( prescale_ == 0 ) prescale_ = 1;
}


AsciiOutputModule::~AsciiOutputModule()
{
  mf::LogAbsolute("AsciiOut")
    << ">>> processed " << counter_ << " events"
    << std::endl;
}


void
  AsciiOutputModule::write( EventPrincipal const & e )
{
  if( (++counter_ % prescale_) != 0 || verbosity_ <= 0 )
    return;

  //  const Run & run = evt.getRun(); // this is still unused
  mf::LogAbsolute("AsciiOut")
    << ">>> processing event # " << e.id() <<" time " << e.time().value()
    << std::endl;

  if( verbosity_ <= 1 )
    return;

  // Write out non-EDProduct contents...

  // ... list of process-names
  for( ProcessHistory::const_iterator it = e.processHistory().begin()
                                    , itEnd = e.processHistory().end()
     ; it != itEnd; ++it ) {
    mf::LogAbsolute("AsciiOut") << it->processName() << " ";
  }

  // ... collision id
  mf::LogAbsolute("AsciiOut") << '\n' << e.id() << '\n';

  // Loop over products, and write some output for each...

  std::vector<Provenance const *> provs;
  e.getAllProvenance(provs);
  for( std::vector<Provenance const *>::const_iterator i = provs.begin()
                                                     , iEnd = provs.end()
     ; i != iEnd; ++i ) {
    BranchDescription const & desc = (*i)->product();
    if( selected(desc) )
      mf::LogAbsolute("AsciiOut") << **i << '\n';
  }

}  // AsciiOutputModule::write()


// ======================================================================


// DEFINE_FWK_MODULE(AsciiOutputModule);
