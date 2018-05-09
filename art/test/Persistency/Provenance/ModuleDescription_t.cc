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
  art::ModuleDescription md3(fhicl::ParameterSet().id(),
                             "class3",
                             "",
                             art::ModuleThreadingType::legacy,
                             art::ProcessConfiguration{});

  art::ModuleDescriptionID id1 = md1.id();
  art::ModuleDescriptionID id2 = md2.id();
  art::ModuleDescriptionID id3 = md3.id();

  assert(id1 != id2);
  assert(id2 != id3);
  assert(id3 != id1);

  art::ModuleDescription md4(fhicl::ParameterSet().id(),
                             "class2",
                             "",
                             art::ModuleThreadingType::legacy,
                             art::ProcessConfiguration{});
  art::ModuleDescriptionID id4 = md4.id();
  assert(md4 == md2);
  assert(id4 != id2);
}
