#include "art/Persistency/Provenance/ModuleDescription.h"
#include "fhiclcpp/ParameterSet.h"

#include <cassert>

int
main()
{
  art::ModuleDescription md1;
  assert(md1 == md1);
  art::ModuleDescription md2;
  assert(md1 == md2);
  md2 = art::ModuleDescription(fhicl::ParameterSet().id(),
                               "class2",
                               "",
                               art::ModuleThreadingType::legacy,
                               art::ProcessConfiguration{});
  art::ModuleDescription md4(fhicl::ParameterSet().id(),
                             "class2",
                             "",
                             art::ModuleThreadingType::legacy,
                             art::ProcessConfiguration{});
  assert(md4 == md2);
}
