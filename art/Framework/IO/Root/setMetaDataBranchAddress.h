#ifndef art_Framework_IO_Root_setMetaDataBranchAddress_h
#define art_Framework_IO_Root_setMetaDataBranchAddress_h

#include "art/Framework/IO/Root/rootNames.h"

#include "TTree.h"

namespace art {
  template <typename T>
  void setMetaDataBranchAddress(TTree *tree,
                                T *&t);
}

template <typename T>
inline
void art::setMetaDataBranchAddress(TTree *tree,
                                   T *&t) {
  tree->SetBranchAddress(rootNames::metaBranchRootName<T>(), &t);
}

#endif /* art_Framework_IO_Root_setMetaDataBranchAddress_h */

// Local Variables:
// mode: c++
// End:
