// ======================================================================
//
// Prescaler_plugin
//
// ======================================================================


// --- Framework support:
#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/MakerMacros.h"
#include "art/ParameterSet/ParameterSet.h"


// --- Contents:
namespace edm {
  class Prescaler;
}
using edm::Prescaler;


// ======================================================================


class edm::Prescaler
  : public EDFilter
{
public:
  explicit Prescaler( ParameterSet const & );
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


Prescaler::Prescaler( ParameterSet const & ps )
  : count_ ( 0 )
  , n_     ( ps.getParameter<int>("prescaleFactor") )
  , offset_( ps.getParameter<int>("prescaleOffset") )
{ }


Prescaler::~Prescaler()
{ }


bool Prescaler::filter( Event & )
{
  return ++count_ % n_ == offset_;
}


void Prescaler::endJob()
{ }


// ======================================================================


DEFINE_FWK_MODULE(Prescaler);
