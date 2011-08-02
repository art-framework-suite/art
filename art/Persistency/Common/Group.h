#ifndef art_Persistency_Common_Group_h
#define art_Persistency_Common_Group_h

/*----------------------------------------------------------------------

Group: A collection of information related to a single EDProduct. This
is the storage unit of such information.

----------------------------------------------------------------------*/

#include "Reflex/Type.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/EDProductGetter.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib/value_ptr.h"
#include "cpp0x/memory"

namespace art {
  class Group;

  void swap(Group &a, Group &b);
  std::ostream &operator<<(std::ostream &os, Group const &g);
}

class art::Group
  : public EDProductGetter
{
  // non-copyable:
  Group(Group const &);
  void operator = (Group const &);

public:
  Group();
  Group(BranchDescription const& bd, ProductID const& pid, bool demand=false);
  Group(std::auto_ptr<EDProduct> edp,
        BranchDescription const& bd,
        ProductID const& pid);
  virtual ~Group();

  void swap(Group& other);

  // product is not available (dropped or never created)
  bool productUnavailable() const;

  // Scheduled for on-demand production
  bool onDemand() const { return onDemand_; }

  EDProduct const *product() const { return product_.get(); }
  EDProduct const* getIt() const { resolveProductIfAvailable(true); return product(); }

  cet::exempt_ptr<ProductProvenance const> productProvenancePtr() const;

  BranchDescription const& productDescription() const {return *branchDescription_;}

  std::string const& moduleLabel() const {return branchDescription_->moduleLabel();}

  std::string const& productInstanceName() const {return branchDescription_->productInstanceName();}

  std::string const& processName() const {return branchDescription_->processName();}

  ProductStatus status() const;

  void setResolvers(BranchMapper  const &bm,
                    DelayedReader const &dr) { ppResolver_.reset(&bm);
                                               productResolver_.reset(&dr); }
  
  bool resolveProduct(bool fillOnDemand) const;
  bool resolveProductIfAvailable(bool fillOnDemand) const;

  // The following is const because we can add an EDProduct to the
  // cache after creation of the Group, without changing the meaning
  // of the Group.
  void setProduct(std::auto_ptr<EDProduct> prod) const;

  // Write the group to the stream.
  void write(std::ostream& os) const;

  // Replace the existing group with a new one
  void replace(Group& g);

  // Return the type of the product stored in this Group.
  // We are relying on the fact that Type instances are small, and
  // so we are free to copy them at will.
  Reflex::Type productType() const;

  void mergeGroup(Group * newGroup);

  ProductID const& productID() const {return pid_;};

private:
  bool dropped() const;

  cet::exempt_ptr<BranchMapper const>       ppResolver_;
  cet::exempt_ptr<DelayedReader const>      productResolver_;
  mutable cet::value_ptr<EDProduct>         product_;
  cet::exempt_ptr<BranchDescription const>  branchDescription_;
  mutable ProductID                         pid_;
  bool                                      onDemand_;

};  // Group

// Free swap function
inline
void
art::swap( art::Group& a, art::Group& b ) {
  a.swap(b);
}

inline
std::ostream&
art::operator << ( std::ostream& os, art::Group const& g ) {
  g.write(os);
  return os;
}

#endif /* art_Persistency_Common_Group_h */

// Local Variables:
// mode: c++
// End:
