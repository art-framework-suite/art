// ======================================================================
//
// EmptySource_plugin
//
// ======================================================================


#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/GeneratedInputSource.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceMacros.h"

#include "fhiclcpp/ParameterSet.h"
  using fhicl::ParameterSet;

// Contents:
namespace art {
  class EmptySource;
}
using art::EmptySource;


// ======================================================================


class art::EmptySource
  : public GeneratedInputSource
{
public:
  explicit EmptySource( ParameterSet           const &
                      , InputSourceDescription const & );
  ~EmptySource();

private:
  virtual bool produce( Event & );

};  // EmptySource


// ======================================================================


EmptySource::EmptySource( ParameterSet           const & pset
                        , InputSourceDescription const & desc )
  : GeneratedInputSource( pset, desc )
{ }


EmptySource::~EmptySource()
{ }


bool
  EmptySource::produce( Event & )
{
  return true;
}


// ======================================================================


DEFINE_FWK_INPUT_SOURCE(EmptySource);
