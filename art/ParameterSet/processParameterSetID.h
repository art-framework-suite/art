#ifndef  PARAMETERSET__PROCESSPARAMETERSETID_H
#define  PARAMETERSET__PROCESSPARAMETERSETID_H

// ======================================================================
//
// processParameterSet
//
// ======================================================================


#include "fhiclcpp/ParameterSetID.h"


namespace art  {

  inline  fhicl::ParameterSetID &
    processParameterSetID( )
  {
    static  fhicl::ParameterSetID  id;
    return id;
  }

}  // namespace art


// ======================================================================


#endif  // PARAMETERSET__PROCESSPARAMETERSETID_H
