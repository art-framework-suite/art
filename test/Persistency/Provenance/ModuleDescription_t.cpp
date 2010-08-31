#include "art/Persistency/Provenance/ModuleDescription.h"
#include <assert.h>

int main()
{
  edm::ModuleDescription md1;
  assert(md1 == md1);
  edm::ModuleDescription md2;
  assert(md1 == md2);
  md2.moduleName_ = "class2";
  edm::ModuleDescription md3;
  md3.moduleName_ = "class3";

  edm::ModuleDescriptionID id1 = md1.id();
  edm::ModuleDescriptionID id2 = md2.id();
  edm::ModuleDescriptionID id3 = md3.id();

  assert(id1 != id2);
  assert(id2 != id3);
  assert(id3 != id1);

  edm::ModuleDescription md4;
  md4.moduleName_ = "class2";
  edm::ModuleDescriptionID id4 = md4.id();
  assert(md4 == md2);
  assert (id4 == id2);
}
