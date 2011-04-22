#ifndef art_Framework_IO_Root_setMetaDataBranchAddress_h
#define art_Framework_IO_Root_setMetaDataBranchAddress_h

#include "art/Utilities/Exception.h"
#include "art/Utilities/TypeID.h"
#include "art/Persistency/Provenance/BranchChildren.h"
#include "art/Persistency/Provenance/BranchIDList.h"
#include "art/Persistency/Provenance/ParameterSetMap.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"

#include "TTree.h"

namespace art {
#define ART_ROOTNAME(T,N) \
  template <> \
  inline char const *rootName<T>() { return N; }

#define ART_ROOTNAME_SIMPLE(T) \
  ART_ROOTNAME(T,#T)

  template <typename T>
  char const *rootName() {
    throw Exception(errors::LogicError)
      << "art::rootName requires a specialization for type "
      << TypeID(typeid(T)).className()
      << "\n";
  }

  class FileFormatVersion;
  ART_ROOTNAME_SIMPLE(FileFormatVersion);
  class FileIndex;
  ART_ROOTNAME_SIMPLE(FileIndex);
  class ProductRegistry;
  ART_ROOTNAME_SIMPLE(ProductRegistry);
  ART_ROOTNAME_SIMPLE(ParameterSetMap);
  ART_ROOTNAME_SIMPLE(ProcessHistoryMap);
  ART_ROOTNAME_SIMPLE(BranchIDLists);
  ART_ROOTNAME(BranchChildren,"ProductDependencies");

  template <typename T>
  void setMetaDataBranchAddress(TTree *tree,
                                T *&t);
}

template <typename T>
inline
void art::setMetaDataBranchAddress(TTree *tree,
                                   T *&t) {
  tree->SetBranchAddress(rootName<T>(), &t);
}

#undef ART_ROOTNAME_SIMPLE
#undef ART_ROOTNAME

#endif /* art_Framework_IO_Root_setMetaDataBranchAddress_h */

// Local Variables:
// mode: c++
// End:
