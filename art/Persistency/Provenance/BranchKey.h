#ifndef art_Persistency_Provenance_BranchKey_h
#define art_Persistency_Provenance_BranchKey_h

/*----------------------------------------------------------------------

BranchKey: The key used to identify a Group in the EventPrincipal. The
name of the branch to which the related data product will be written
is determined entirely from the BranchKey.

----------------------------------------------------------------------*/

#include "art/Persistency/Provenance/BranchType.h"

#include <iosfwd>
#include <string>

namespace art {
  class BranchDescription;

  struct BranchKey {
    BranchKey() :
      friendlyClassName_(),
      moduleLabel_(),
      productInstanceName_(),
      processName_(),
      branchType_(NumBranchTypes)
    {}

    explicit BranchKey(std::string const& cn, std::string const& ml,
                       std::string const& pin, std::string const& pn,
                       BranchType const bt) :
      friendlyClassName_(cn),
      moduleLabel_(ml),
      productInstanceName_(pin),
      processName_(pn),
      branchType_(bt)
    {}

    explicit BranchKey(BranchDescription const& desc);

    std::string friendlyClassName_;
    std::string moduleLabel_;
    std::string productInstanceName_;
    std::string processName_; // ???
    int branchType_;
  };

  inline
  bool
  operator<(BranchKey const& a, BranchKey const& b) {
    return
      a.friendlyClassName_ < b.friendlyClassName_ ? true :
      a.friendlyClassName_ > b.friendlyClassName_ ? false :
      a.moduleLabel_ < b.moduleLabel_ ? true :
      a.moduleLabel_ > b.moduleLabel_ ? false :
      a.productInstanceName_ < b.productInstanceName_ ? true :
      a.productInstanceName_ > b.productInstanceName_ ? false :
      a.processName_ < b.processName_ ? true :
      a.processName_ > b.processName_ ? false :
      a.branchType_ < b.branchType_ ? true :
      false;
  }

  inline
  bool
  operator==(BranchKey const& a, BranchKey const& b) {
    return !(a < b || b < a);
  }

  inline
  bool
  operator!=(BranchKey const& a, BranchKey const& b) {
    return !(a == b);
  }

  std::ostream&
  operator<<(std::ostream& os, BranchKey const& bk);
}
#endif /* art_Persistency_Provenance_BranchKey_h */

// Local Variables:
// mode: c++
// End:
