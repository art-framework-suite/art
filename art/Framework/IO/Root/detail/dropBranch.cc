#include "art/Framework/IO/Root/detail/dropBranch.h"
// vim: set sw=2:

#include "TBranch.h"
#include "TLeaf.h"
#include "TObjArray.h"

void
art::detail::dropBranch(TTree* tree, std::string const& branchName)
{
  TBranch* branch = tree->GetBranch(branchName.c_str());
  if (!branch) {
    return;
  }
  TObjArray* leaves = tree->GetListOfLeaves();
  int entries = leaves->GetEntries();
  for (int i = 0; i < entries; ++i) {
    auto leaf = reinterpret_cast<TLeaf*>((*leaves)[i]);
    if (leaf == nullptr) {
      continue;
    }
    TBranch* br = leaf->GetBranch();
    if (br == nullptr) {
      continue;
    }
    if (br->GetMother() == branch) {
      leaves->Remove(leaf);
    }
  }
  leaves->Compress();
  tree->GetListOfBranches()->Remove(branch);
  tree->GetListOfBranches()->Compress();
  delete branch;
}
