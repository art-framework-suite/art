#ifndef art_Framework_Principal_Principal_h
#define art_Framework_Principal_Principal_h
// vim: set sw=2:

//
//  Principal
//
//  Pure abstract base class for Run-, SubRun-, and EventPrincipal,
//  the classes which manage data products.
//
//  The major internal component is the Group, which contains an EDProduct
//  and its associated Provenance, along with ancillary transient information
//  regarding the two.  Groups are handled through shared pointers.
//
//  The Principal returns GroupQueryResult, rather than a shared
//  pointer to a Group, when queried.
//

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/OutputHandle.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Common/Wrapper.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/ProcessHistory.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/ProductStatus.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"
#include "art/Utilities/InputTag.h"
#include "art/Utilities/TypeID.h"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/memory"
#include <map>
#include <string>
#include <vector>
#include <cstdio>

namespace art {

class Principal {

public: // TYPES

  using GroupCollection = std::map<BranchID, std::shared_ptr<Group>>;
  using const_iterator = GroupCollection::const_iterator;
  using ProcessNameConstIterator = ProcessHistory::const_iterator;
  using SharedConstGroupPtr = std::shared_ptr<const Group>;
  using GroupQueryResultVec = std::vector<GroupQueryResult>;
  using size_type = GroupCollection::size_type;
  using SharedGroupPtr = std::shared_ptr<Group>;
  using ProcessName = std::string;

public: // MEMBER FUNCTIONS

  virtual
  ~Principal();

  Principal(Principal const&) = delete;

  Principal&
  operator=(Principal const&) = delete;

  Principal(ProcessConfiguration const&, ProcessHistoryID const&,
            std::unique_ptr<BranchMapper>&&, std::unique_ptr<DelayedReader>&&,
            int idx, Principal*);

  OutputHandle
  getForOutput(BranchID const&, bool resolveProd) const;

  GroupQueryResult
  getBySelector(TypeID const&, SelectorBase const&) const;

  GroupQueryResult
  getByLabel(TypeID const&, std::string const& label,
             std::string const& productInstanceName,
             std::string const& processName) const;

  void
  getMany(TypeID const&, SelectorBase const&,
          std::vector<GroupQueryResult>& results) const;

  void
  getManyByType(TypeID const&, std::vector<GroupQueryResult>& results) const;

  // Return a vector of GroupQueryResults to the products which:
  //   1. are sequences,
  //   2. and have the nested type 'value_type'
  //   3. and for which elementType is the same as or a public base of
  //      this value_type,
  //   4. and which matches the given selector

  size_t
  getMatchingSequence(TypeID const& elementType, SelectorBase const&,
                      std::vector<GroupQueryResult>& results,
                      bool stopIfProcessHasMatch) const;

  void
  removeCachedProduct(BranchID const& bid) const
  {
    getExistingGroup(bid)->removeCachedProduct();
  }

  // FIXME: Unused!
  void
  setSecondaryPrincipals(std::vector<std::unique_ptr<Principal>>& sp)
  {
    secondaryPrincipals_.clear();
    for (auto I = sp.begin(), E = sp.end(); I != E; ++I) {
      secondaryPrincipals_.emplace_back(std::move(*I));
    }
  }

  // FIXME: Unused!
  void
  setSecondaryPrincipals(std::vector<std::shared_ptr<Principal>> const& sp)
  {
    secondaryPrincipals_.clear();
    secondaryPrincipals_.insert(secondaryPrincipals_.end(), sp.begin(),
                                sp.end());
  }

  void
  addSecondaryPrincipal(std::unique_ptr<Principal>&& val)
  {
    secondaryPrincipals_.emplace_back(std::move(val));
  }

  void
  addSecondaryPrincipal(std::shared_ptr<Principal> const& val)
  {
    secondaryPrincipals_.emplace_back(val);
  }

  void
  readImmediate() const
  {
    readProvenanceImmediate();
    for (auto const& val : groups_) {
      if (!val.second->productUnavailable()) {
        val.second->resolveProduct(false, val.second->producedWrapperType());
      }
    }
  }

  void
  readProvenanceImmediate() const
  {
    for (auto const& val : groups_) {
      if (!val.second->onDemand()) {
        (void) val.second->productProvenancePtr();
      }
    }
    branchMapperPtr_->setDelayedRead(false);
  }

  ProcessHistory const&
  processHistory() const
  {
    return *processHistoryPtr_;
  }

  ProcessConfiguration const&
  processConfiguration() const
  {
    return processConfiguration_;
  }

  BranchMapper const&
  branchMapper() const
  {
    return *branchMapperPtr_;
  }

  size_t
  size() const
  {
    return groups_.size();
  }

  const_iterator
  begin() const
  {
    return groups_.begin();
  }

  const_iterator
  cbegin() const
  {
    return groups_.cbegin();
  }

  const_iterator
  end() const
  {
    return groups_.end();
  }

  const_iterator
  cend() const
  {
    return groups_.cend();
  }

  // Flag that we have been updated in the current process.
  void
  addToProcessHistory() const;

  // Obtain the branch type suitable for products inserted into the principal.
  virtual
  BranchType
  branchType() const = 0;

  virtual
  void
  addGroup(BranchDescription const&) = 0;

  virtual
  void
  addGroup(std::unique_ptr<EDProduct>&&, BranchDescription const&) = 0;

protected: // MEMBER FUNCTIONS

  BranchMapper&
  branchMapper()
  {
    return *branchMapperPtr_;
  }

  DelayedReader&
  productReader()
  {
    return *store_;
  }

  // Add a new Group.
  // We take ownership of the Group, which in turn owns its data.
  void
  addGroup_(std::unique_ptr<Group>&& group)
  {
    BranchDescription const& bd = group->productDescription();
    assert(!bd.producedClassName().empty());
    assert(!bd.friendlyClassName().empty());
    assert(!bd.moduleLabel().empty());
    assert(!bd.processName().empty());
    group->setResolvers(branchMapper(), *store_);
    std::shared_ptr<Group> g(group.release());
    groups_.insert(make_pair(bd.branchID(), g));
  }

  void
  replaceGroup(std::unique_ptr<Group>&& group)
  {
    BranchDescription const& bd = group->productDescription();
    assert(!bd.producedClassName().empty());
    assert(!bd.friendlyClassName().empty());
    assert(!bd.moduleLabel().empty());
    assert(!bd.processName().empty());
    group->setResolvers(branchMapper(), *store_);
    std::shared_ptr<Group> g(group.release());
    groups_[bd.branchID()]->replace(*g);
  }

  int
  tryNextSecondaryFile() const;

  cet::exempt_ptr<Group const>
  getExistingGroup(BranchID const& bid) const;

  std::shared_ptr<const Group> const
  getGroupForPtr(BranchType const btype, BranchID const bid) const;

  std::shared_ptr<const Group> const
  getGroup(BranchID const& bid) const;

  std::shared_ptr<const Group> const
  getResolvedGroup(BranchID const& bid, bool resolveProd,
                   bool fillOnDemand) const;

private: // TYPES

  // FIXME: Is this to break a circular reference with mpr?
  using ProcessLookup = std::map<std::string const, std::vector<BranchID>>;
  using TypeLookup = std::map<std::string const, ProcessLookup>;

private: // MEMBER FUNCTIONS

  virtual
  void
  addOrReplaceGroup(std::unique_ptr<Group>&&) = 0;

  virtual
  ProcessHistoryID const&
  processHistoryID() const = 0;

  virtual
  void
  setProcessHistoryID(ProcessHistoryID const&) const = 0;

  size_t
  findGroupsForProduct(TypeID const& wanted_product, SelectorBase const&,
                       std::vector<GroupQueryResult>& results,
                       bool stopIfProcessHasMatch) const;

  size_t
  findGroups(ProcessLookup const&, SelectorBase const&,
             std::vector<GroupQueryResult>& results,
             bool stopIfProcessHasMatch,
             TypeID wanted_wrapper = TypeID()) const;

  void
  findGroupsForProcess(std::vector<BranchID> const& vbid,
                       SelectorBase const& selector,
                       std::vector<GroupQueryResult>& results,
                       TypeID wanted_wrapper) const;

private: // MEMBER DATA

  std::shared_ptr<ProcessHistory> processHistoryPtr_;

  ProcessConfiguration const& processConfiguration_;

  mutable bool processHistoryModified_;

  // products and provenances are persistent
  std::map<BranchID, std::shared_ptr<Group>> groups_;

  // Pointer to the mapper that will get provenance
  // information from the persistent store.
  std::unique_ptr<BranchMapper> branchMapperPtr_;

  // Pointer to the reader that will be used to obtain
  // EDProducts from the persistent store.
  std::unique_ptr<DelayedReader> store_;

  // Back pointer to the primary principal.
  cet::exempt_ptr<Principal> primaryPrincipal_;

  // Secondary principals.  Note that the lifetime of run
  // and subRun principals is the lifetime of the input file,
  // while the lifetime of event principals ends at the next
  // event read.
  // FIXME: Is this true, or do run and subRun principals
  // FIXME: have the lifetime of the job?
  std::vector<std::shared_ptr<Principal>> secondaryPrincipals_;

  // Index into the per-file lookup tables.  Each principal is
  // read from particular secondary file.
  int secondaryIdx_;

  // Index into the secondary file names vector of the next
  // file that a secondary principal should be created from.
  mutable int nextSecondaryFileIdx_;

};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif // art_Framework_Principal_Principal_h
