#ifndef art_Framework_Principal_Principal_h
#define art_Framework_Principal_Principal_h

/*----------------------------------------------------------------------

Principal: This is the implementation of the classes responsible
for management of EDProducts. It is not seen by reconstruction code.

The major internal component of the Principal is the Group, which
contains an EDProduct and its associated Provenance, along with
ancillary transient information regarding the two. Groups are handled
through shared pointers.

The Principal returns GroupQueryResult, rather than a shared
pointer to a Group, when queried.

(Historical note: prior to April 2007 this class was named DataBlockImpl)

----------------------------------------------------------------------*/

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
#include "boost/noncopyable.hpp"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/memory"
#include <map>
#include <string>
#include <vector>

class art::Principal
  : public boost::noncopyable
{
public:
  typedef std::map<BranchID, std::shared_ptr<Group> > GroupCollection;
  typedef GroupCollection::const_iterator const_iterator;
  typedef ProcessHistory::const_iterator ProcessNameConstIterator;
  typedef std::shared_ptr<const Group> SharedConstGroupPtr;
  typedef std::vector<GroupQueryResult> GroupQueryResultVec;
  typedef GroupCollection::size_type      size_type;

  typedef std::shared_ptr<Group> SharedGroupPtr;
  typedef std::string ProcessName;

  Principal(ProcessConfiguration const& pc,
            ProcessHistoryID const& hist,
            std::unique_ptr<BranchMapper> && mapper,
            //std::shared_ptr<DelayedReader> rtrv);
            std::unique_ptr<DelayedReader> && rtrv);

  virtual ~Principal();

  ////    EDProductGetter const* prodGetter() const {return this;}

  OutputHandle getForOutput(BranchID const& bid, bool getProd) const;

  GroupQueryResult  getBySelector(TypeID const& tid,
                                  SelectorBase const& s) const;

  GroupQueryResult  getByLabel(TypeID const& tid,
                               std::string const& label,
                               std::string const& productInstanceName,
                               std::string const& processName) const;

  void getMany(TypeID const& tid,
               SelectorBase const&,
               GroupQueryResultVec& results) const;

  void getManyByType(TypeID const& tid,
                     GroupQueryResultVec& results) const;

  // Return a vector of GroupQueryResults to the products which:
  //   1. are sequences,
  //   2. and have the nested type 'value_type'
  //   3. and for which elementType is the same as or a public base of
  //      this value_type,
  //   4. and which matches the given selector
  size_t getMatchingSequence(TypeID const& elementType,
                             SelectorBase const& selector,
                             GroupQueryResultVec& results,
                             bool stopIfProcessHasMatch) const;

  void
  readImmediate() const;

  void
  readProvenanceImmediate() const;

  ProcessHistory const& processHistory() const;

  ProcessConfiguration const& processConfiguration() const {return processConfiguration_;}

  BranchMapper const &branchMapper() const {return *branchMapperPtr_;}

  // ----- Mark this Principal as having been updated in the
  // current Process.
  void addToProcessHistory() const;

  size_t  size() const { return groups_.size(); }

  const_iterator begin() const {return groups_.begin();}
  const_iterator end() const {return groups_.end();}

  // Obtain the branch type suitable for products to be put in the
  // principal.
  virtual BranchType branchType() const = 0;

protected:
  // ----- Add a new Group
  // *this takes ownership of the Group, which in turn owns its
  // data.
  void addGroup_(std::unique_ptr<Group> && g);
  cet::exempt_ptr<Group const>  getExistingGroup(BranchID const &bid);
  void replaceGroup(std::unique_ptr<Group> && g);
  SharedConstGroupPtr const getGroup(BranchID const& bid) const;
  SharedConstGroupPtr const
  getResolvedGroup(BranchID const& bid,
                   bool resolveProd,
                   bool fillOnDemand) const;
  BranchMapper &branchMapper() {return *branchMapperPtr_;}

  DelayedReader &productReader() { return *store_; }

private:
  ////    virtual EDProduct const* getIt(ProductID const&) const;

  virtual void addOrReplaceGroup(std::unique_ptr<Group> && g) = 0;


  virtual ProcessHistoryID const& processHistoryID() const = 0;

  virtual void setProcessHistoryID(ProcessHistoryID const& phid) const = 0;

  // Used for indices to find groups by type and process
  typedef std::map<std::string, std::vector<BranchID> > ProcessLookup;
  typedef std::map<std::string, ProcessLookup> TypeLookup;

  size_t
  findGroupsForProduct(TypeID const& wanted_product,
                       SelectorBase const& selector,
                       GroupQueryResultVec& results,
                       bool stopIfProcessHasMatch) const;

  size_t
  findGroupsForElement(TypeID const& wanted_element,
                       TypeLookup const& typeLookup,
                       SelectorBase const& selector,
                       GroupQueryResultVec& results,
                       bool stopIfProcessHasMatch) const;

  size_t findGroups(ProcessLookup const& processLookup,
                    SelectorBase const& selector,
                    GroupQueryResultVec& results,
                    bool stopIfProcessHasMatch,
                    TypeID wanted_wrapper = TypeID()) const;

  void findGroupsForProcess(std::string const& processName,
                            ProcessLookup const& processLookup,
                            SelectorBase const& selector,
                            GroupQueryResultVec& results,
                            TypeID wanted_wrapper) const;

  std::shared_ptr<ProcessHistory> processHistoryPtr_;

  ProcessConfiguration const& processConfiguration_;

  mutable bool processHistoryModified_;

  GroupCollection groups_; // products and provenances are persistent

  // Pointer to the 'mapper' that will get provenance information
  // from the persistent store.
  std::auto_ptr<BranchMapper> branchMapperPtr_;

  // Pointer to the 'source' that will be used to obtain EDProducts
  // from the persistent store.
  std::auto_ptr<DelayedReader> store_;
};

#endif /* art_Framework_Principal_Principal_h */

// Local Variables:
// mode: c++
// End:
