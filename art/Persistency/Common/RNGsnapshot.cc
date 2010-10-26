// ======================================================================
//
// RNGsnapshot
//
// ======================================================================


// Framework support:
#include "art/Persistency/Common/RNGsnapshot.h"
#include "art/Utilities/vectorTransform.h"
  using art::RNGsnapshot;


// ======================================================================


// --- C'tor/d'tor:
RNGsnapshot::RNGsnapshot( )
  : label_( )
  , state_( )
{ }

RNGsnapshot::~RNGsnapshot( )
{ }

// --- Save/restore:
void
  RNGsnapshot::saveFrom( std::string    const & ekind
                       , label_t        const & lbl
                       , engine_state_t const & est
                       )
{
  engine_kind_ = ekind;
  label_ = lbl;
  vectorTransform<CLHEP_t,saved_t>( est, state_ );
}

void
  RNGsnapshot::restoreTo( engine_state_t & est ) const
{
  vectorTransform<saved_t,CLHEP_t>( state_, est );
}
