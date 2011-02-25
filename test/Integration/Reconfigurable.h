#ifndef RECONFIGU_H
#define RECONFIGU_H 1
// ======================================================================
//
// ======================================================================

#include "art/Framework/Core/Event.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "fhiclcpp/ParameterSet.h"

#include "messagefacility/MessageLogger/MessageLogger.h"

// ----------------------------------------------------------------------

class Reconfigurable
{
public:
  Reconfigurable(fhicl::ParameterSet const& cfg, art::ActivityRegistry& ar);
  
  void reconfigure(fhicl::ParameterSet const& cfg);
  int get_debug_level() { return debug_level_; }

private:
  int debug_level_;
  int other_value_;

};

#endif


