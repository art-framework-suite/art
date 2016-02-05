#ifndef art_Persistency_Provenance_BranchKey_h
#define art_Persistency_Provenance_BranchKey_h

/*----------------------------------------------------------------------

BranchKey: The key used to identify a Group in the EventPrincipal. The
name of the branch to which the related data product will be written
is determined entirely from the BranchKey.

----------------------------------------------------------------------*/

#include "canvas/Persistency/Provenance/BranchType.h"

#include <iosfwd>
#include <string>

namespace art {
  class BranchDescription;

  struct BranchKey;

  bool operator < (BranchKey const & a, BranchKey const & b);
  bool operator == (BranchKey const & a, BranchKey const & b);
  bool operator != (BranchKey const & a, BranchKey const & b);

  std::ostream&
  operator<<(std::ostream& os, BranchKey const& bk);
}

struct art::BranchKey {
  BranchKey();
  BranchKey(std::string cn, std::string ml,
            std::string pin, std::string pn,
            BranchType bt = NumBranchTypes);

  explicit BranchKey(BranchDescription const& desc);

  std::string friendlyClassName_;
  std::string moduleLabel_;
  std::string productInstanceName_;
  std::string processName_;
  int branchType_;
};

#ifndef __GCCXML__
inline
art::BranchKey::
BranchKey() :
  friendlyClassName_(),
  moduleLabel_(),
  productInstanceName_(),
  processName_(),
  branchType_(NumBranchTypes)
{}

inline
art::BranchKey::
BranchKey(std::string cn, std::string ml,
          std::string pin, std::string pn,
          BranchType const bt) :
  friendlyClassName_(std::move(cn)),
  moduleLabel_(std::move(ml)),
  productInstanceName_(std::move(pin)),
  processName_(std::move(pn)),
  branchType_(bt)
{
}

inline
bool
art::operator<(BranchKey const& a, BranchKey const& b) {
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
art::operator==(BranchKey const& a, BranchKey const& b) {
  return !(a < b || b < a);
}

inline
bool
art::operator!=(BranchKey const& a, BranchKey const& b) {
  return !(a == b);
}

#endif /* __GCCXML__ */

#endif /* art_Persistency_Provenance_BranchKey_h */

// Local Variables:
// mode: c++
// End:
