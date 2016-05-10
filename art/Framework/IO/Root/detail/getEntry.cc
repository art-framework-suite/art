#include "art/Framework/IO/Root/Inputfwd.h"
// vim: set sw=2:

#include "TBranch.h"
#include "TTree.h"
#include "canvas/Utilities/Exception.h"

namespace art {
  namespace input {

    Int_t
    getEntry(TBranch* branch, EntryNumber entryNumber) try
      {
        return branch->GetEntry(entryNumber);
      }
    catch (cet::exception& e)
      {
        throw art::Exception(art::errors::FileReadError) << e.explain_self() << "\n";
      }

    Int_t
    getEntry(TTree* tree, EntryNumber entryNumber) try
      {
        return tree->GetEntry(entryNumber);
      }
    catch (cet::exception& e)
      {
        throw art::Exception(art::errors::FileReadError) << e.explain_self() << "\n";
      }

  } // namespace input
} // namespace art
