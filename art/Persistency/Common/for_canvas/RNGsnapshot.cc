// ======================================================================
//
// RNGsnapshot
//
// ======================================================================

#include "art/Persistency/Common/RNGsnapshot.h"
#include "cetlib/container_algorithms.h"

using art::RNGsnapshot;

// ======================================================================

namespace {

  template< class From, class To >
  inline To
  cast_one( From value )
  {
    return static_cast<To>( value );
  }

}

// --- Save/restore:
void
RNGsnapshot::
saveFrom( std::string    const & ekind,
          label_t        const & lbl,
          engine_state_t const & est )
{
  engine_kind_ = ekind;
  label_ = lbl;
  cet::transform_all( est, std::back_inserter(state_), cast_one<CLHEP_t,saved_t> );
}

void
RNGsnapshot::
restoreTo( engine_state_t & est ) const
{
  cet::transform_all( state_, std::back_inserter(est), cast_one<saved_t,CLHEP_t> );
}

// ======================================================================
