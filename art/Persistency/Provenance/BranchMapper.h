#ifndef art_Persistency_Provenance_BranchMapper_h
#define art_Persistency_Provenance_BranchMapper_h

// ======================================================================
//
// BranchMapper: Manages the per event/subRun/run per product provenance.
//
// ======================================================================

#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib/value_ptr.h"
#include "cpp0x/memory"

#include <iosfwd>
#include <map>
#include <set>

namespace art {
  // defined below:
  class BranchMapper;
  std::ostream &operator << (std::ostream &, BranchMapper const &);

  // forward declaration:
  class ProductID;
}

// ----------------------------------------------------------------------

class art::BranchMapper {
public:
  BranchMapper(BranchMapper const&) = delete;
  BranchMapper& operator=(BranchMapper const&) = delete;

  typedef  cet::exempt_ptr<ProductProvenance const>  result_t;

  explicit BranchMapper(bool delayedRead = false);
  virtual ~BranchMapper() { }

  void write(std::ostream &) const;

  result_t branchToProductProvenance(BranchID const &) const;

#ifndef __GCCXML__
  result_t insert(std::unique_ptr<ProductProvenance const>&& );
#endif
  void setDelayedRead(bool value) {delayedRead_ = value;}

private:
  typedef std::map <BranchID, cet::value_ptr<ProductProvenance const> >  eiSet;

  eiSet         entryInfoSet_;
  mutable bool  delayedRead_;

  void readProvenance() const;
  virtual void readProvenance_() const { }

};  // BranchMapper

inline
std::ostream &
art::operator << (std::ostream &os, BranchMapper const &p)
{
  p.write(os);
  return os;
}

// ======================================================================

#endif /* art_Persistency_Provenance_BranchMapper_h */

// Local Variables:
// mode: c++
// End:
