#ifndef art_Framework_IO_Root_detail_dropBranch_h
#define art_Framework_IO_Root_detail_dropBranch_h

#include "TTree.h"
#include <string>

namespace art {
  namespace detail {
    void dropBranch(TTree* tree, std::string const& branchName);
  }
}

#endif
