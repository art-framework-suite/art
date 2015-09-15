#ifndef art_Persistency_Provenance_BranchID_h
#define art_Persistency_Provenance_BranchID_h

/*----------------------------------------------------------------------

BranchID: A unique identifier for each branch.

----------------------------------------------------------------------*/

#include <iosfwd>
#include <string>

namespace art {
  class BranchID {
  public:
    typedef unsigned int value_type;
    BranchID() : id_(0) { }
    explicit BranchID(std::string const& branchName) : id_(toID(branchName)) {
    }
    explicit BranchID(value_type id) : id_(id) {
    }
    void setID(std::string const& branchName) {id_ = toID(branchName);}
    unsigned int id() const { return id_; }
    bool isValid() const {return id_ != 0;}

    bool operator<(BranchID const& rh) const {return id_ < rh.id_;}
    bool operator>(BranchID const& rh) const {return id_ > rh.id_;}
    bool operator==(BranchID const& rh) const {return id_ == rh.id_;}
    bool operator!=(BranchID const& rh) const {return id_ != rh.id_;}

#ifndef __GCCXML__
    struct Hash {
      std::size_t operator()(BranchID const& bid) const
      {
        return bid.id(); // since the ID is already a checksum, don't
                         // worry about further hashing
      }
    };
#endif

  private:
    static value_type toID(std::string const& branchName);
    value_type id_;
  };

  std::ostream&
  operator<<(std::ostream& os, BranchID const& id);
}
#endif /* art_Persistency_Provenance_BranchID_h */

// Local Variables:
// mode: c++
// End:
