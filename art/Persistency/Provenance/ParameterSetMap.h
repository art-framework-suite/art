#ifndef ART_PERSISTENCY_PROVENANCE_PARAMETERSETMAP_H
#define ART_PERSISTENCY_PROVENANCE_PARAMETERSETMAP_H
#include "art/Persistency/Provenance/ParameterSetBlob.h"
#include "fhiclcpp/ParameterSetID.h"
#include <map>

namespace art {
   typedef std::map<fhicl::ParameterSetID, ParameterSetBlob> ParameterSetMap;
}

#endif
