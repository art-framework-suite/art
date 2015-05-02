#ifndef art_Utilities_ParameterSetHelpers_CLHEP_ps_h
#define art_Utilities_ParameterSetHelpers_CLHEP_ps_h

#include "fhiclcpp/coding.h"
#include "CLHEP/Vector/TwoVector.h"
#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Vector/LorentzVector.h"

namespace CLHEP {

  inline void decode( boost::any const & a, Hep2Vector & result ) {
    std::array<double,2> tmp;
    fhicl::detail::decode(a,tmp);
    result.setX( tmp.at(0) );
    result.setY( tmp.at(1) );
  }

  inline void decode( boost::any const & a, Hep3Vector & result ) {
    std::array<double,3> tmp;
    fhicl::detail::decode(a,tmp);
    result.setX( tmp.at(0) );
    result.setY( tmp.at(1) );
    result.setZ( tmp.at(2) );
  }

  inline void decode( boost::any const & a, HepLorentzVector & result ) {
    std::array<double,4> tmp;
    fhicl::detail::decode(a,tmp);
    result.setX( tmp.at(0) );
    result.setY( tmp.at(1) );
    result.setZ( tmp.at(2) );
    result.setT( tmp.at(3) );
  }

}

// Local variables:
// mode: c++
// End:

#endif // art_Utilities_ParameterSetHelpers_CLHEP_ps_h
