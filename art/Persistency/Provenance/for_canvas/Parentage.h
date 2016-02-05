#ifndef art_Persistency_Provenance_Parentage_h
#define art_Persistency_Provenance_Parentage_h

/*----------------------------------------------------------------------

Parentage: The products that were read in producing this product.

definitions:
Product: The EDProduct to which a provenance object is associated
Parents: The EDProducts used as input by the creator.

----------------------------------------------------------------------*/

#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/ParentageID.h"
#include <iosfwd>
#include <vector>

namespace art {
  class Parentage {
  public:
    Parentage();

    // use compiler-generated copy c'tor, copy assignment, and d'tor

    // Only the 'salient attributes' are encoded into the ID.
    ParentageID id() const;

    void write(std::ostream& os) const;

    std::vector<BranchID> const& parents() const {return parents_;}
    std::vector<BranchID> & parents() {return parents_;}

  private:
    // The Branch IDs of the parents
    std::vector<BranchID> parents_;

  };

  inline
  std::ostream&
  operator<<(std::ostream& os, Parentage const& p) {
    p.write(os);
    return os;
  }

  // Only the 'salient attributes' are testing in equality comparison.
  bool operator==(Parentage const& a, Parentage const& b);
  inline bool operator!=(Parentage const& a, Parentage const& b) { return !(a==b); }
}
#endif /* art_Persistency_Provenance_Parentage_h */

// Local Variables:
// mode: c++
// End:
