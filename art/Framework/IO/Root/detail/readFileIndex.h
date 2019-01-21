#ifndef art_Framework_IO_Root_detail_readFileIndex_h
#define art_Framework_IO_Root_detail_readFileIndex_h

#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Framework/IO/Root/rootErrMsgs.h"
#include "canvas/Persistency/Provenance/FileIndex.h"
#include "canvas/Persistency/Provenance/rootNames.h"

#include "TFile.h"
#include "TTree.h"

#include <memory>

// This function selects retrieves the FileIndex based on whether it
// is a branch (file format < 7) or tree (file format >= 7).

namespace art {
  namespace detail {

    inline void
    readFileIndex(TFile* file, TTree* metaDataTree, FileIndex*& findexPtr)
    {

      if (auto branch = metaDataTree->GetBranch(
            rootNames::metaBranchRootName<FileIndex>())) {
        branch->SetAddress(&findexPtr);
        input::getEntry(branch, 0);
        branch->ResetAddress();
      } else {
        std::unique_ptr<TTree> fileIndexTree{static_cast<TTree*>(
          file->Get(rootNames::fileIndexTreeName().c_str()))};
        if (!fileIndexTree) {
          throw Exception{errors::FileReadError}
            << couldNotFindTree(rootNames::fileIndexTreeName());
        }
        FileIndex::Element element;
        auto elemPtr = &element;
        fileIndexTree->SetBranchAddress(
          rootNames::metaBranchRootName<FileIndex::Element>(), &elemPtr);
        for (size_t i{0}, sz = fileIndexTree->GetEntries(); i != sz; ++i) {
          input::getEntry(fileIndexTree.get(), i);
          findexPtr->addEntryOnLoad(elemPtr->eventID_, elemPtr->entry_);
        }
        {
          auto br = fileIndexTree->GetBranch(
            rootNames::metaBranchRootName<FileIndex::Element>());
          br->ResetAddress();
        }
      }
    }
  } // namespace detail
} // namespace art

#endif /* art_Framework_IO_Root_detail_readFileIndex_h */

// Local Variables:
// mode: c++
// End:
