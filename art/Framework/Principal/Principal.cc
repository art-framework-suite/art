#include "art/Framework/Principal/Principal.h"

#include "art/Persistency/Common/DelayedReader.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/ProcessHistory.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Persistency/Provenance/ProductStatus.h"
#include "art/Persistency/Provenance/ReflexTools.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/TypeID.h"
#include "art/Utilities/WrappedClassName.h"
#include "cetlib/container_algorithms.h"
#include "cpp0x/algorithm"
#include "cpp0x/utility"

#include <sstream>
#include <stdexcept>

using namespace cet;
using namespace std;

namespace {
  cet::exempt_ptr<art::MasterProductRegistry::ProcessLookup const>
  findProcessLookup(art::TypeID const& typeID,
                    art::MasterProductRegistry::TypeLookup const& typeLookup)
  {
    // A class without a dictionary cannot be in an Event/SubRun/Run.
    // First, we check if the class has a dictionary.  If it does not,
    // we return immediately.  This is necessary to avoid an exception
    // being thrown inside TypeID::friendlyClassName().
    if (typeID.hasDictionary()) {
      art::MasterProductRegistry::TypeLookup::const_iterator i =
        typeLookup.find(typeID.friendlyClassName());
      if (i != typeLookup.end()) {
        return cet::exempt_ptr<art::MasterProductRegistry::ProcessLookup const>(&i->second);
      }
    }
    return cet::exempt_ptr<art::MasterProductRegistry::ProcessLookup const>();
  }
}

namespace art {

  Principal::Principal(ProcessConfiguration const& pc,
                       ProcessHistoryID const& hist,
                       std::unique_ptr<BranchMapper> && mapper,
                       std::unique_ptr<DelayedReader> && rtrv) :
    processHistoryPtr_(std::shared_ptr<ProcessHistory>(new ProcessHistory)),
    processConfiguration_(pc),
    processHistoryModified_(false),
    groups_(),
    branchMapperPtr_(mapper.release()),
    store_(rtrv.release())
  {
    if (hist.isValid()) {
      assert(! ProcessHistoryRegistry::empty());
      bool found __attribute__((unused)) = ProcessHistoryRegistry::get(hist, *processHistoryPtr_);
      assert(found);
    }
  }

  Principal::~Principal() {
  }

  cet::exempt_ptr<Group const>
  Principal::getExistingGroup(BranchID const &bid) {
    GroupCollection::const_iterator it = groups_.find(bid);
    return (it == groups_.end())?
      cet::exempt_ptr<Group const>():
      cet::exempt_ptr<Group const>(it->second.get());
  }

  void
  Principal::addGroup_(std::unique_ptr<Group> && group) {
    BranchDescription const& bd = group->productDescription();
    assert (!bd.producedClassName().empty());
    assert (!bd.friendlyClassName().empty());
    assert (!bd.moduleLabel().empty());
    assert (!bd.processName().empty());
    group->setResolvers(branchMapper(), *store_);
    SharedGroupPtr g(group.release());
    groups_.insert(make_pair(bd.branchID(), g));
  }

  void
  Principal::replaceGroup(std::unique_ptr<Group> && group) {
    BranchDescription const& bd = group->productDescription();
    assert (!bd.producedClassName().empty());
    assert (!bd.friendlyClassName().empty());
    assert (!bd.moduleLabel().empty());
    assert (!bd.processName().empty());
    group->setResolvers(branchMapper(), *store_);
    SharedGroupPtr g(group.release());
    groups_[bd.branchID()]->replace(*g);
  }

  void
  Principal::addToProcessHistory() const {
    if (processHistoryModified_) return;
    ProcessHistory& ph = *processHistoryPtr_;
    string const& processName = processConfiguration_.processName();
    for (ProcessHistory::const_iterator
           it = ph.begin(),
           itEnd = ph.end(); it != itEnd; ++it) {
      if (processName == it->processName()) {
        throw art::Exception(errors::Configuration)
          << "The process name " << processName
          << " was previously used on these products.\n"
          "Please modify the configuration file to use a distinct process name.\n";
      }
    }
    ph.push_back(processConfiguration_);
    //OPTIMIZATION NOTE:  As of 0_9_0_pre3
    // For very simple Sources (e.g. EmptyEvent) this routine takes up nearly 50% of the time per event.
    // 96% of the time for this routine is being spent in computing the
    // ProcessHistory id which happens because we are reconstructing the ProcessHistory for each event.
    // (The process ID is first computed in the call to 'insertMapped(..)' below.)
    // It would probably be better to move the ProcessHistory construction out to somewhere
    // which persists for longer than one Event
    ProcessHistoryRegistry::put(ph);
    setProcessHistoryID(ph.id());
    processHistoryModified_ = true;
  }

  ProcessHistory const&
  Principal::processHistory() const {
    return *processHistoryPtr_;
  }

  Principal::SharedConstGroupPtr const
  Principal::getGroup(BranchID const& bid) const {
    GroupCollection::const_iterator it = groups_.find(bid);
    if (it == groups_.end()) {
      return SharedConstGroupPtr();
    } else {
      return it->second;
    }
  }

  Principal::SharedConstGroupPtr const
  Principal::getResolvedGroup(BranchID const& bid,
                              bool resolveProd,
                              bool fillOnDemand) const {
    // FIXME: This reproduces the behavior of the original getGroup with
    // resolveProv == false but I'm not sure this is correct in the face
    // of an unavailable product.
    SharedConstGroupPtr const& g(getGroup(bid));
    if (g.get() &&
        resolveProd &&
        !g->resolveProductIfAvailable(fillOnDemand, g->producedWrapperType()) &&
        g->onDemand()) {
      // Behavior is the same as if the group wasn't there.
      return SharedConstGroupPtr();
    } else {
      return g;
    }
  }

  GroupQueryResult
  Principal::getBySelector(TypeID const& productType,
                           SelectorBase const& sel) const {

    GroupQueryResultVec results;

    int nFound = findGroupsForProduct(productType,
                                      sel,
                                      results,
                                      true);

    if (nFound == 0) {
      std::shared_ptr<cet::exception> whyFailed( new art::Exception(art::errors::ProductNotFound) );
      *whyFailed
        << "getBySelector: Found zero products matching all criteria\n"
        << "Looking for type: " << productType << "\n";
      return GroupQueryResult(whyFailed);
    }
    if (nFound > 1) {
      throw art::Exception(art::errors::ProductNotFound)
        << "getBySelector: Found "<<nFound<<" products rather than one which match all criteria\n"
        "Looking for type: " << productType << "\n";
    }
    return results[0];
  }

  GroupQueryResult
  Principal::getByLabel(TypeID const& productType,
                        string const& label,
                        string const& productInstanceName,
                        string const& processName) const
  {

    GroupQueryResultVec results;

    art::Selector sel(art::ModuleLabelSelector(label) &&
                      art::ProductInstanceNameSelector(productInstanceName) &&
                      art::ProcessNameSelector(processName));

    int nFound = findGroupsForProduct(productType,
                                      sel,
                                      results,
                                      true);

    if (nFound == 0) {
      std::shared_ptr<cet::exception> whyFailed( new art::Exception(art::errors::ProductNotFound) );
      *whyFailed
        << "getByLabel: Found zero products matching all criteria\n"
        "Looking for type: " << productType << "\n"
        "Looking for module label: " << label << "\n"
        "Looking for productInstanceName: " << productInstanceName << "\n"
        << (processName.empty() ? "" : "Looking for process: ") << processName << "\n";
      return GroupQueryResult(whyFailed);
    }
    if (nFound > 1) {
      throw art::Exception(art::errors::ProductNotFound)
        << "getByLabel: Found "<<nFound<<" products rather than one which match all criteria\n"
        "Looking for type: " << productType << "\n"
        "Looking for module label: " << label << "\n"
        "Looking for productInstanceName: " << productInstanceName << "\n"
        << (processName.empty() ? "" : "Looking for process: ") << processName << "\n";
    }
    return results[0];
  }


  void
  Principal::getMany(TypeID const& productType,
                     SelectorBase const& sel,
                     GroupQueryResultVec& results) const {

    findGroupsForProduct(productType,
                         sel,
                         results,
                         false);

    return;
  }

  void
  Principal::getManyByType(TypeID const& productType,
                           GroupQueryResultVec& results) const {

    art::MatchAllSelector sel;

    findGroupsForProduct(productType,
                         sel,
                         results,
                         false);
    return;
  }

  size_t
  Principal::getMatchingSequence(TypeID const& elementType,
                                 SelectorBase const& selector,
                                 GroupQueryResultVec& results,
                                 bool stopIfProcessHasMatch) const {

    // One new argument is the element lookup container
    // Otherwise this just passes through the arguments to findGroups
    return findGroupsForElement(elementType,
                                ProductMetaData::instance().elementLookup(),
                                selector,
                                results,
                                stopIfProcessHasMatch);
  }

  void
  Principal::readImmediate() const {
    readProvenanceImmediate();
    for (Principal::const_iterator i = begin(), iEnd = end(); i != iEnd; ++i) {
      if (!i->second->productUnavailable()) {
        i->second->resolveProduct(false, i->second->producedWrapperType());
      }
    }
  }

  void
  Principal::readProvenanceImmediate() const {
    for (Principal::const_iterator i = begin(), iEnd = end(); i != iEnd; ++i) {
      if ( ! i->second->onDemand()) {
        (void) i->second->productProvenancePtr();
      }
    }
    branchMapperPtr_->setDelayedRead(false);
  }

  size_t
  Principal::findGroupsForProduct(TypeID const& wanted_product,
                                  SelectorBase const& selector,
                                  GroupQueryResultVec& results,
                                  bool stopIfProcessHasMatch) const
  {
    TypeLookup const& typeLookup = ProductMetaData::instance().productLookup();
    cet::exempt_ptr<ProcessLookup const>
      pl(findProcessLookup(wanted_product, typeLookup));
    if (pl) {
      Reflex::Type rt(Reflex::Type::ByName(wrappedClassName(wanted_product.persistentClassName())));
      if (!rt) {
        throw Exception(errors::DictionaryNotFound)
          << "Dictionary not found for "
          << wrappedClassName(wanted_product.persistentClassName())
          << ".\n";
      }
      return findGroups(*pl,
                        selector,
                        results,
                        stopIfProcessHasMatch,
                        TypeID(rt.TypeInfo())
                       );
    }
    return 0;
  }

  size_t
  Principal::findGroupsForElement(TypeID const& wanted_element,
                                  TypeLookup const& typeLookup,
                                  SelectorBase const& selector,
                                  GroupQueryResultVec& results,
                                  bool stopIfProcessHasMatch) const
  {
    cet::exempt_ptr<ProcessLookup const>
      pl(findProcessLookup(wanted_element, typeLookup));
    return pl ? findGroups(*pl, selector, results, stopIfProcessHasMatch) :
      0;
  }

  size_t
  Principal::findGroups(ProcessLookup const& processLookup,
                        SelectorBase const& selector,
                        GroupQueryResultVec& results,
                        bool stopIfProcessHasMatch,
                        TypeID wanted_wrapper) const {
    // Handle groups for current process, note that we need to
    // look at the current process even if it is not in the processHistory
    // because of potential unscheduled (onDemand) production
    findGroupsForProcess(processConfiguration_.processName(),
                         processLookup,
                         selector,
                         results,
                         wanted_wrapper);

    // Loop over processes in reverse time order.  Sometimes we want to stop
    // after we find a process with matches so check for that at each step.
    for (ProcessHistory::const_reverse_iterator iproc = processHistory().rbegin(),
                                                eproc = processHistory().rend();
         iproc != eproc && (results.empty() || !stopIfProcessHasMatch);
         ++iproc) {

      // We just dealt with the current process before the loop so skip it
      if (iproc->processName() == processConfiguration_.processName()) continue;

      findGroupsForProcess(iproc->processName(),
                           processLookup,
                           selector,
                           results,
                           wanted_wrapper);
    }
    return results.size();
  }

  void
  Principal::findGroupsForProcess(string const& processName,
                                  ProcessLookup const& processLookup,
                                  SelectorBase const& selector,
                                  GroupQueryResultVec& results,
                                  TypeID wanted_wrapper) const {

    ProcessLookup::const_iterator j = processLookup.find(processName);

    if (j == processLookup.end()) return;

    // This is a vector of indexes into the productID vector
    // These indexes point to groups with desired process name (and
    // also type when this function is called from findGroups)
    vector<BranchID> const& vindex = j->second;

    for (vector<BranchID>::const_iterator ib(vindex.begin()), ie(vindex.end());
         ib != ie;
         ++ib) {
      SharedConstGroupPtr const& group = getGroup(*ib);
      if(group.get() == 0) {
        continue;
      }

      if (selector.match(group->productDescription())) {

        // Skip product if not available.
        if (!group->productUnavailable()) {
          group->resolveProduct(true,
                                wanted_wrapper ? wanted_wrapper : group->producedWrapperType());
          // If the product is a dummy filler, group will now be marked unavailable.
          // Unscheduled execution can fail to produce the EDProduct so check
          if (!group->productUnavailable() && !group->onDemand()) {
            // Found a good match, save it
            results.push_back( GroupQueryResult(group.get()) );
          }
        }
      }
    }
  }

  OutputHandle
  Principal::getForOutput(BranchID const& bid, bool getProd) const {
    SharedConstGroupPtr const &g =
      getResolvedGroup(bid, getProd, false);
    if (!g) {
      return OutputHandle();
    }
    if (getProd && (g->anyProduct() == 0 || !g->anyProduct()->isPresent()) &&
        g->productDescription().present() &&
        g->productDescription().branchType() == InEvent &&
        productstatus::present(g->productProvenancePtr()->productStatus())) {
      throw Exception(errors::LogicError, "Principal::getForOutput\n")
        << "A product with a status of 'present' is not actually present.\n"
        "The branch name is " << g->productDescription().branchName() << "\n"
        "Contact a framework developer.\n";
    }
    if (!g->anyProduct() && !g->productProvenancePtr()) {
      return OutputHandle();
    }
    return OutputHandle(g->anyProduct(), &g->productDescription(), g->productProvenancePtr());
  }
}
