#ifndef  PARAMETERSET_PROCESSPARAMETERSETID_H
#define  PARAMETERSET_PROCESSPARAMETERSETID_H

// ======================================================================
//
// processParameterSet
//
// ======================================================================

#include "fhiclcpp/ParameterSetID.h"

// ----------------------------------------------------------------------

namespace art  {

  inline  fhicl::ParameterSetID &
    processParameterSetID( )
  {
    static  fhicl::ParameterSetID  id;
    return id;
  }

}  // art

// ======================================================================

#endif
