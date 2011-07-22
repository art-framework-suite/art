#ifndef art_Persistency_Provenance_ConstBranchDescription_h
#define art_Persistency_Provenance_ConstBranchDescription_h

/*----------------------------------------------------------------------

ConstBranchDescription - A class containing a constant shareable branch
                         description that is inexpensive to copy.

This class is not persistable.

----------------------------------------------------------------------*/

#include "Reflex/Type.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "cpp0x/memory"
#include <iosfwd>
#include <set>
#include <string>

// ----------------------------------------------------------------------

namespace art {

  class ConstBranchDescription
  {
  public:
    explicit ConstBranchDescription(BranchDescription const& bd) :
      ptr_(new BranchDescription(bd)) {}

    void write(std::ostream& os) const {ptr_->write(os);}

    std::string const& moduleLabel() const {return ptr_->moduleLabel();}
    std::string const& processName() const {return ptr_->processName();}
    BranchID const& branchID() const {return ptr_->branchID();}
    std::string const& fullClassName() const {return ptr_->fullClassName();}
    std::string const& className() const {return ptr_->fullClassName();}
    std::string const& friendlyClassName() const {return ptr_->friendlyClassName();}
    std::string const& productInstanceName() const {return ptr_->productInstanceName();}
    bool const& produced() const {return ptr_->produced();}
    bool const& present() const {return ptr_->present();}
    bool const& transient() const {return ptr_->transient();}
    int const& splitLevel() const {return ptr_->splitLevel();}
    int const& basketSize() const {return ptr_->basketSize();}

    fhicl::ParameterSetID const& parameterSetID() const {return ptr_->parameterSetID();}
    std::set<fhicl::ParameterSetID> const& psetIDs() const {return ptr_->psetIDs();}
    std::set<std::string> const& branchAliases() const {return ptr_->branchAliases();}
    std::string const& branchName() const {return ptr_->branchName();}
    BranchType const& branchType() const {return ptr_->branchType();}
    std::string const& wrappedName() const {return ptr_->wrappedName();}
    std::string const& wrappedCintName() const {return ptr_->wrappedCintName();}

    BranchDescription const& me() const {return *ptr_;}

  private:
    std::shared_ptr<BranchDescription const> ptr_;
  };  // ConstBranchDescription

  inline
  std::ostream&
  operator<<(std::ostream& os, ConstBranchDescription const& p) {
    os << p.me();
    return os;
  }

  inline
  bool operator<(ConstBranchDescription const& a, ConstBranchDescription const& b) {
    return a.me() < b.me();
  }

  inline
  bool operator==(ConstBranchDescription const& a, ConstBranchDescription const& b) {
    return a.me() == b.me();
  }

  inline
  std::string match(ConstBranchDescription const& a,
        ConstBranchDescription const& b,
        std::string const& fileName,
        BranchDescription::MatchMode m) {
    return match(a.me(), b.me(), fileName, m);
  }

}  // art

// ======================================================================

#endif /* art_Persistency_Provenance_ConstBranchDescription_h */

// Local Variables:
// mode: c++
// End:
