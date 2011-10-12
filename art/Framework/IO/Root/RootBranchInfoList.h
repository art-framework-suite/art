#ifndef art_Framework_IO_Root_RootBranchInfoList_h
#define art_Framework_IO_Root_RootBranchInfoList_h

#include "art/Framework/IO/Root/RootBranchInfo.h"
#include "art/Utilities/fwd.h"
#include "art/Utilities/TypeID.h"

#include <vector>

#include "TTree.h"

namespace art {
  class RootBranchInfoList;
}

class art::RootBranchInfoList {
public:
  RootBranchInfoList();
  explicit RootBranchInfoList(TTree *tree);
  void reset(TTree *tree);

  template <class PROD>
  bool findBranchInfo(InputTag const &tag, RootBranchInfo &rbInfo) const;

  bool findBranchInfo(TypeID const &type,
                      InputTag const &tag,
                      RootBranchInfo &rbInfo) const;
private:
  typedef std::vector<RootBranchInfo> Data_t;
  Data_t data_;
};

template <class PROD>
bool
art::RootBranchInfoList::findBranchInfo(InputTag const &tag,
                                        RootBranchInfo &rbInfo) const {
  return findBranchInfo(TypeID(typeid(PROD)),
                        tag,
                        rbInfo);
}

#endif /* art_Framework_IO_Root_RootBranchInfoList_h */

// Local Variables:
// mode: c++
// End:
