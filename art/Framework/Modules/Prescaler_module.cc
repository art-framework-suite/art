// ======================================================================
//
// Prescaler_plugin
//
// ======================================================================


#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "fhiclcpp/ParameterSet.h"

namespace art {
  class Prescaler;
}
using art::Prescaler;

// ======================================================================

class art::Prescaler
  : public EDFilter
{
public:
  explicit Prescaler( fhicl::ParameterSet const & );
  virtual ~Prescaler();

  virtual bool filter( Event & );
  void endJob();

private:
  int count_;
  int n_;      // accept one in n
  int offset_; // with offset,
               // i.e. sequence of events does not have to start at first event

};  // Prescaler

// ======================================================================

Prescaler::Prescaler( fhicl::ParameterSet const & ps )
  : EDFilter( )
  , count_ ( 0 )
  , n_     ( ps.get<int>("prescaleFactor") )
  , offset_( ps.get<int>("prescaleOffset") )
{ }

Prescaler::~Prescaler()
{ }

bool
  Prescaler::filter( Event & /* unused */ )
{
  return ++count_ % n_ == offset_;
}

void
  Prescaler::endJob()
{ }

// ======================================================================

DEFINE_ART_MODULE(Prescaler)

// ======================================================================
