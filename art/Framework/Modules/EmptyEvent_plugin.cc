// ======================================================================
//
// EmptyEvent_plugin
//
// ======================================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/GeneratedInputSource.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "fhiclcpp/ParameterSet.h"

namespace art {
  class EmptyEvent;
}
using art::EmptyEvent;
using fhicl::ParameterSet;

// ======================================================================

class art::EmptyEvent
  : public GeneratedInputSource
{
public:
  explicit EmptyEvent( ParameterSet           const &
                      , InputSourceDescription const & );
  ~EmptyEvent();

private:
  virtual bool produce( Event & );

};  // EmptyEvent

// ======================================================================

EmptyEvent::EmptyEvent( ParameterSet           const & pset
                        , InputSourceDescription const & desc
                        )
  : GeneratedInputSource( pset, desc )
{ }

EmptyEvent::~EmptyEvent()
{ }

bool
  EmptyEvent::produce( Event & )
{
  return true;
}

// ======================================================================

DEFINE_FWK_INPUT_SOURCE(EmptyEvent);

// ======================================================================
