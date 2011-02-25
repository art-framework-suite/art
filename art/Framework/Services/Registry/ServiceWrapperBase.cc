// ======================================================================
//
// ServiceWrapperBase - Base class through which the framework manages
//                      the lifetime of ServiceWrapper<T> objects.
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceWrapperBase.h"

art::ServiceWrapperBase::~ServiceWrapperBase( )
{ }

void art::ServiceWrapperBase::reconfigure(fhicl::ParameterSet const&) 
{ }

// ======================================================================
