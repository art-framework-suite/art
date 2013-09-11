#ifndef QQQJJSDHSKDAJT
#define QQQJJSDHSKDAJT

#include "fhiclcpp/ParameterSet.h"

namespace art {
  class EvProcInitHelper;
}

class art::EvProcInitHelper {
public:
  EvProcInitHelper(fhicl::ParameterSet const & ps);
  fhicl::ParameterSet const & servicesPS() const;
  fhicl::ParameterSet const & schedulerPS() const;
private:
  fhicl::ParameterSet const servicesPS_;
  fhicl::ParameterSet const schedulerPS_;
};


inline
fhicl::ParameterSet const &
art::EvProcInitHelper::
servicesPS() const
{
  return servicesPS_;
}

inline
fhicl::ParameterSet const &
art::EvProcInitHelper::
schedulerPS() const
{
  return schedulerPS_;
}

#endif
