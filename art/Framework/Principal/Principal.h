#ifndef art_Framework_Principal_Principal_h
#define art_Framework_Principal_Principal_h
// vim: set sw=2:

// =================================================================
// Principal
//
// Pure abstract base class for Run-, SubRun-, and EventPrincipal,
// the classes which manage data products.
//
// The major internal component is the Group, which contains an
// EDProduct and its associated Provenance, along with ancillary
// transient information regarding the two.  Groups are handled
// through shared pointers.
//
// The Principal returns GroupQueryResult, rather than a shared
// pointer to a Group, when queried.
// =================================================================

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/OutputHandle.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "canvas/Persistency/Common/EDProductGetterFinder.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Provenance/BranchMapper.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Utilities/TypeID.h"
#include "canvas/Utilities/WrappedTypeID.h"
#include "cetlib/exempt_ptr.h"

#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace art {

  class Principal : public EDProductGetterFinder {

  public: // TYPES
    using GroupCollection = std::map<ProductID, std::unique_ptr<Group>>;
    using const_iterator = GroupCollection::const_iterator;
    using ProcessNameConstIterator = ProcessHistory::const_iterator;
    using GroupQueryResultVec = std::vector<GroupQueryResult>;
    using size_type = GroupCollection::size_type;
    using ProcessName = std::string;

  public: // MEMBER FUNCTIONS
    virtual ~Principal() noexcept = default;

    // Disable copying
    Principal(Principal const&) = delete;
    Principal& operator=(Principal const&) = delete;

    Principal(ProcessConfiguration const&,
              ProcessHistoryID const&,
              cet::exempt_ptr<ProductTable const> presentProducts,
              std::unique_ptr<BranchMapper>&&,
              std::unique_ptr<DelayedReader>&&);

    EDProductGetter const* productGetter(ProductID const pid) const;

    OutputHandle getForOutput(ProductID const, bool resolveProd) const;

    GroupQueryResult getBySelector(WrappedTypeID const& wrapped,
                                   SelectorBase const&) const;

    GroupQueryResult getByProductID(ProductID const pid) const;

    GroupQueryResult getByLabel(WrappedTypeID const& wrapped,
                                std::string const& label,
                                std::string const& productInstanceName,
                                std::string const& processName) const;

    GroupQueryResultVec getMany(WrappedTypeID const& wrapped,
                                SelectorBase const&) const;

    // ROOT-FIXME: Return a vector of GroupQueryResults to the products which:
    //   1. are sequences,
    //   2. and have the nested type 'value_type'
    //   3. and for which elementType is the same as or a public base of
    //      this value_type,
    //   4. and which matches the given selector

    GroupQueryResultVec getMatchingSequence(SelectorBase const&) const;

    void removeCachedProduct(ProductID const pid) const;

    void
    addSecondaryPrincipal(std::unique_ptr<Principal>&& val)
    {
      secondaryPrincipals_.emplace_back(std::move(val));
    }

    void
    setProducedProducts(ProductTable const& producedProducts)
    {
      producedProducts_ = cet::make_exempt_ptr(&producedProducts);
    }

    void
    readImmediate() const
    {
      readProvenanceImmediate();
      for (auto const& val : groups_) {
        if (!val.second->productUnavailable()) {
          val.second->resolveProduct(val.second->producedWrapperType());
        }
      }
    }

    void
    readProvenanceImmediate() const
    {
      for (auto const& val : groups_) {
        (void)val.second->productProvenancePtr();
      }
      branchMapperPtr_->setDelayedRead(false);
    }

    ProcessHistory const&
    processHistory() const
    {
      return processHistory_;
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
    void addToProcessHistory();

    // Obtain the branch type suitable for products inserted into the
    // principal.
    virtual BranchType branchType() const = 0;

    virtual void fillGroup(BranchDescription const&) = 0;

    virtual RangeSet seenRanges() const = 0;

    virtual bool parentageEnabled() const = 0;

    virtual bool rangesEnabled() const = 0;

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

    // We take ownership of the Group, which in turn owns its data.
    void
    fillGroup(std::unique_ptr<Group>&& group)
    {
      BranchDescription const& pd = group->productDescription();
      assert(!pd.producedClassName().empty());
      assert(!pd.friendlyClassName().empty());
      assert(!pd.moduleLabel().empty());
      assert(!pd.processName().empty());
      group->setResolvers(branchMapper(), *store_);
      groups_[pd.productID()] = std::move(group);
    }

    int tryNextSecondaryFile() const;

    cet::exempt_ptr<Group const> getGroupForPtr(ProductID const pid) const;

    cet::exempt_ptr<Group const> getGroup(ProductID const pid) const;

    cet::exempt_ptr<Group const> getResolvedGroup(ProductID const pid,
                                                  bool resolveProd) const;

  private: // MEMBER FUNCTIONS
    virtual ProcessHistoryID const& processHistoryID() const = 0;

    virtual void setProcessHistoryID(ProcessHistoryID const&) = 0;

    GroupQueryResultVec matchingSequenceFromInputFile(
      SelectorBase const&) const;

    GroupQueryResultVec findGroupsForProduct(WrappedTypeID const& wrapped,
                                             SelectorBase const&,
                                             bool stopIfProcessHasMatch) const;

    size_t findGroupsFromInputFile(WrappedTypeID const& wrapped,
                                   SelectorBase const&,
                                   GroupQueryResultVec& results,
                                   bool stopIfProcessHasMatch) const;

    size_t findGroups(ProcessLookup const&,
                      SelectorBase const&,
                      GroupQueryResultVec& results,
                      bool stopIfProcessHasMatch,
                      TypeID wanted_wrapper = TypeID{}) const;

    size_t findGroupsForProcess(std::vector<ProductID> const& vpid,
                                SelectorBase const& selector,
                                GroupQueryResultVec& results,
                                TypeID wanted_wrapper) const;

    bool presentFromSource(ProductID) const;

  private: // MEMBER DATA
    // This function and its associated member datum are required to
    // handle the lifetime of a deferred getter, which in turn is
    // required because a group does not exist until it is placed in
    // the event.
    EDProductGetter const* deferredGetter_(ProductID const pid) const;

    EDProductGetter const*
    getEDProductGetterImpl(ProductID const pid) const final override
    {
      return getByProductID(pid).result().get();
    }

    ProcessHistory processHistory_{};
    static ProcessHistory previousProcessHistory_;
    static ProcessHistory pendingProcessHistory_;
    static int pendingProcessHistorySet_;

    ProcessConfiguration const& processConfiguration_;
    cet::exempt_ptr<ProductTable const> presentProducts_;
    cet::exempt_ptr<ProductTable const> producedProducts_{nullptr};

    mutable std::map<ProductID, std::shared_ptr<DeferredProductGetter const>>
      deferredGetters_{};

    mutable bool processHistoryModified_{false};

    // Products and provenances are persistent.
    GroupCollection groups_{};

    // Pointer to the mapper that will get provenance information from
    // the persistent store.
    std::unique_ptr<BranchMapper> branchMapperPtr_;

    // Pointer to the reader that will be used to obtain EDProducts
    // from the persistent store.
    std::unique_ptr<DelayedReader> store_;

    // Secondary principals.  Note that the lifetime of run and subRun
    // principals is the lifetime of the input file, while the
    // lifetime of event principals ends at the next event read.
    std::vector<std::unique_ptr<Principal>> secondaryPrincipals_{};

    // Index into the secondary file names vector of the next file
    // that a secondary principal should be created from.
    mutable int nextSecondaryFileIdx_{};
  };

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_Principal_Principal_h */
