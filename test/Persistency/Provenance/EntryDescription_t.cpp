#include "art/Persistency/Provenance/Parentage.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ModuleDescriptionID.h"
#include "art/Persistency/Provenance/BranchID.h"
#include <assert.h>

int main()
{
  art::Parentage ed1;
  assert(ed1 == ed1);
  art::Parentage ed2;
  assert(ed1 == ed2);

  ed2.parents() = std::vector<art::BranchID>(1);
  art::Parentage ed3;
  ed3.parents() = std::vector<art::BranchID>(2);

  art::ParentageID id1 = ed1.id();
  art::ParentageID id2 = ed2.id();
  art::ParentageID id3 = ed3.id();

  assert(id1 != id2);
  assert(ed1 != ed2);
  assert(id1 != id3);
  assert(ed1 != ed3);
  assert(id2 != id3);
  assert(ed2 != ed3);

  art::Parentage ed4;
  ed4.parents() = std::vector<art::BranchID>(1);
  art::ParentageID id4 = ed4.id();
  assert(ed4 == ed2);
  assert (id4 == id2);
}
