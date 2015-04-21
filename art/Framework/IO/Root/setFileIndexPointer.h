#ifndef art_Framework_IO_Root_setFileIndexPointer_h
#define art_Framework_IO_Root_setFileIndexPointer_h

#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Framework/IO/Root/rootNames.h"
#include "art/Framework/IO/Root/rootErrMsgs.h"
#include "art/Framework/IO/Root/setMetaDataBranchAddress.h"
#include "art/Persistency/Provenance/FileIndex.h"

#include "TFile.h"
#include "TTree.h"

// This function selects retrieves the FileIndex based on whether it
// is a branch (file format < 7) or tree (file format >= 7).

namespace art {

  inline
  void setFileIndexPointer(TFile* file, TTree* metaDataTree, FileIndex *& findexPtr) {

    if (metaDataTreeHasBranchFor<FileIndex>(metaDataTree)) {
      setMetaDataBranchAddress(metaDataTree, findexPtr);
    }
    else {
      TTree* fileIndexTree = dynamic_cast<TTree*>(file->Get(rootNames::fileIndexTreeName().c_str()));
      if (!fileIndexTree)
        throw art::Exception(errors::FileReadError) << couldNotFindTree(rootNames::fileIndexTreeName());

      FileIndex::Element* elemPtr = nullptr;
      setMetaDataBranchAddress(fileIndexTree, elemPtr);

      for (size_t i(0), sz = fileIndexTree->GetEntries(); i != sz ; ++i) {
        input::getEntry(fileIndexTree,i);
        findexPtr->addEntry(elemPtr->eventID_, elemPtr->entry_);
      }
    }
  }

}

#endif /* art_Framework_IO_Root_setFileIndexPointer_h */

// Local Variables:
// mode: c++
// End:
