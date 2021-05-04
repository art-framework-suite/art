// ======================================================================
//
// MyService
//
// ======================================================================

#include "MyService.h"

#include "art/Framework/Services/Registry/ServiceDefinitionMacros.h"

DEFINE_ART_SERVICE_INTERFACE_IMPL(art::test::MyService,
                                  art::test::MyServiceInterface)
