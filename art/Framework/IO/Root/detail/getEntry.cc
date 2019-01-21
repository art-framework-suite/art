#include "art/Framework/IO/Root/Inputfwd.h"
// vim: set sw=2 expandtab :

#include "canvas/Utilities/Exception.h"

#include "TBranch.h"
#include "TTree.h"

namespace art {
  namespace input {

    Int_t
    getEntry(TBranch* branch, EntryNumber entryNumber) try {
      auto ret = branch->GetEntry(entryNumber);
      return ret;
    }
    catch (cet::exception& e) {
      throw art::Exception(art::errors::FileReadError)
        << e.explain_self() << "\n";
    }

    Int_t
    getEntry(TTree* tree, EntryNumber entryNumber) try {
      auto ret = tree->GetEntry(entryNumber);
      return ret;
    }
    catch (cet::exception& e) {
      throw art::Exception(art::errors::FileReadError)
        << e.explain_self() << "\n";
    }

  } // namespace input
} // namespace art
