#include "art/Framework/Principal/EventPrincipal.h"

#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/UnscheduledHandler.h"
#include "art/Persistency/Common/Group.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Common/Provenance.h"
#include "art/Persistency/Provenance/BranchIDList.h"
#include "art/Persistency/Provenance/BranchIDListRegistry.h"
#include "art/Persistency/Provenance/BranchListIndex.h"
#include "cetlib/container_algorithms.h"
#include "cpp0x/algorithm"
#include "cpp0x/utility"


using namespace cet;
using namespace std;


namespace art {
  EventPrincipal::EventPrincipal(EventAuxiliary const& aux,
                                 ProcessConfiguration const& pc,
                                 std::shared_ptr<History> history,
                                 std::auto_ptr<BranchMapper> mapper,
                                 std::shared_ptr<DelayedReader> rtrv) :
  Principal(pc, history->processHistoryID(), mapper, rtrv),
  aux_(aux),
  subRunPrincipal_(),
  unscheduledHandler_(),
  moduleLabelsRunning_(),
  history_(history),
  branchToProductIDHelper_() {
    if (ProductMetaData::instance().productProduced(InEvent)) {
      addToProcessHistory();
      // Add index into BranchIDListRegistry for products produced this process
      history_->addBranchListIndexEntry(BranchIDListRegistry::instance()->size()-1);
    }
    // Fill in helper map for Branch to ProductID mapping
    for (BranchListIndexes::const_iterator
         it = history->branchListIndexes().begin(),
         itEnd = history->branchListIndexes().end();
         it != itEnd; ++it) {
      ProcessIndex pix = it - history->branchListIndexes().begin();
      branchToProductIDHelper_.insert(make_pair(*it, pix));
    }
  }

  SubRunPrincipal const& EventPrincipal::subRunPrincipal() const {
    if (!subRunPrincipal_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL subRunPrincipal.\n";
    }
    return *subRunPrincipal_;
  }

  SubRunPrincipal & EventPrincipal::subRunPrincipal() {
    if (!subRunPrincipal_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL subRunPrincipal.\n";
    }
    return *subRunPrincipal_;
  }

  RunPrincipal const&
  EventPrincipal::runPrincipal() const {
    return subRunPrincipal().runPrincipal();
  }

  RunPrincipal &
  EventPrincipal::runPrincipal() {
    return subRunPrincipal().runPrincipal();
  }

  void
  EventPrincipal::addOnDemandGroup(BranchDescription const& desc) {
    auto_ptr<Group> g(new Group(desc, branchIDToProductID(desc.branchID()), true));
    addOrReplaceGroup(g);
  }

  void
  EventPrincipal::addOrReplaceGroup(auto_ptr<Group> g) {
    Group const* group = getExistingGroup(*g);
    if (group == 0) {
      addGroup_(g);
    } else if(group->onDemand()) {
      replaceGroup(g);
    } else {
      BranchDescription const& bd = group->productDescription();
      throw art::Exception(art::errors::InsertFailure,"AlreadyPresent")
        << "addGroup_: Problem found while adding product provenance, "
        << "product already exists for ("
        << bd.friendlyClassName() << ","
        << bd.moduleLabel() << ","
        << bd.productInstanceName() << ","
        << bd.processName()
        << ")\n";
    }
  }

  void
  EventPrincipal::addGroup(BranchDescription const& bd) {
    auto_ptr<Group> g(new Group(bd, branchIDToProductID(bd.branchID())));
    addOrReplaceGroup(g);
  }

  void
  EventPrincipal::addGroup(auto_ptr<EDProduct> prod,
         BranchDescription const& bd,
         cet::exempt_ptr<ProductProvenance const> productProvenance) {
    auto_ptr<Group> g(new Group(prod, bd, branchIDToProductID(bd.branchID()), productProvenance));
    addOrReplaceGroup(g);
  }

  void
  EventPrincipal::addGroup(BranchDescription const& bd,
         cet::exempt_ptr<ProductProvenance const> productProvenance) {
    auto_ptr<Group> g(new Group(bd, branchIDToProductID(bd.branchID()), productProvenance));
    addOrReplaceGroup(g);
  }

  void
  EventPrincipal::put(auto_ptr<EDProduct> edp,
                BranchDescription const& bd,
                auto_ptr<ProductProvenance const> productProvenance) {

    if (edp.get() == 0) {
      throw art::Exception(art::errors::InsertFailure,"Null Pointer")
        << "put: Cannot put because auto_ptr to product is null.\n";
    }
    ProductID pid = branchIDToProductID(bd.branchID());
    if (!pid.isValid()) {
      throw art::Exception(art::errors::InsertFailure,"Null Product ID")
        << "put: Cannot put product with null Product ID.\n";
    }
    this->addGroup(edp, bd, branchMapper().insert(productProvenance));
  }

  BranchID
  EventPrincipal::productIDToBranchID(ProductID const& pid) const {
    if (!pid.isValid()) {
      throw art::Exception(art::errors::ProductNotFound,"InvalidID")
        << "get by product ID: invalid ProductID supplied\n";
    }
    BranchID::value_type bid = 0;
    try {
      BranchListIndex blix = history().branchListIndexes().at(pid.processIndex()-1);
      BranchIDList const& blist = BranchIDListRegistry::instance()->data().at(blix);
      bid = blist.at(pid.productIndex()-1);
    }
    catch (std::exception) {
      return BranchID();
    }
    return BranchID(bid);
  }

  ProductID
  EventPrincipal::branchIDToProductID(BranchID const& bid) const {
    if (!bid.isValid()) {
      throw art::Exception(art::errors::NotFound,"InvalidID")
        << "branchIDToProductID: invalid BranchID supplied\n";
    }
    BranchIDListHelper::BranchIDToIndexMap const& branchIDToIndexMap =
      BranchIDListRegistry::instance()->extra().branchIDToIndexMap();
    BranchIDListHelper::BranchIDToIndexMap::const_iterator it = branchIDToIndexMap.find(bid);
    if (it == branchIDToIndexMap.end()) {
      throw art::Exception(art::errors::NotFound,"Bad BranchID")
        << "branchIDToProductID: productID cannot be determined from BranchID\n";
    }
    BranchListIndex blix = it->second.first;
    ProductIndex productIndex = it->second.second;
    map<BranchListIndex, ProcessIndex>:: const_iterator i = branchToProductIDHelper_.find(blix);
    if (i == branchToProductIDHelper_.end()) {
      throw art::Exception(art::errors::NotFound,"Bad branch ID")
        << "branchIDToProductID: productID cannot be determined from BranchID\n";
    }
    ProcessIndex processIndex = i->second;
    return ProductID(processIndex+1, productIndex+1);
  }

  GroupQueryResult
  EventPrincipal::getByProductID(ProductID const& pid) const {
    BranchID bid = productIDToBranchID(pid);
    SharedConstGroupPtr const& g = getGroup(bid, true, true, true);
    if (g.get() == 0) {
      std::shared_ptr<cet::exception> whyFailed( new art::Exception(art::errors::ProductNotFound,"InvalidID") );
      *whyFailed
        << "get by product ID: no product with given id: "<< pid << "\n";
      return GroupQueryResult(whyFailed);
    }

    // Check for case where we tried on demand production and
    // it failed to produce the object
    if (g->onDemand()) {
      std::shared_ptr<cet::exception> whyFailed( new art::Exception(art::errors::ProductNotFound,"InvalidID") );
      *whyFailed
        << "get by product ID: no product with given id: " << pid << "\n"
        << "onDemand production failed to produce it.\n";
      return GroupQueryResult(whyFailed);
    }
    else
      return GroupQueryResult(g.get());
  }

  EDProduct const *
  EventPrincipal::getIt(ProductID const& pid) const {
    return getByProductID(pid).result()->product();
  }

  void
  EventPrincipal::setUnscheduledHandler(std::shared_ptr<UnscheduledHandler> iHandler) {
    unscheduledHandler_ = iHandler;
  }

  EventSelectionIDVector const&
  EventPrincipal::eventSelectionIDs() const
  {
    return history_->eventSelectionIDs();
  }

  bool
  EventPrincipal::unscheduledFill(string const& moduleLabel) const {

    // If it is a module already currently running in unscheduled
    // mode, then there is a circular dependency related to which
    // EDProducts modules require and produce.  There is no safe way
    // to recover from this.  Here we check for this problem and throw
    // an exception.
    vector<string>::const_iterator i =
      find_in_all(moduleLabelsRunning_, moduleLabel);

    if (i != moduleLabelsRunning_.end()) {
      throw art::Exception(errors::LogicError)
        << "Hit circular dependency while trying to run an unscheduled module.\n"
        << "Current implementation of unscheduled execution cannot always determine\n"
        << "the proper order for module execution.  It is also possible the modules\n"
        << "have a built in circular dependence that will not work with any order.\n"
        << "In the first case, scheduling some or all required modules in paths will help.\n"
        << "In the second case, the modules themselves will have to be fixed.\n";
    }

    moduleLabelsRunning_.push_back(moduleLabel);

    if (unscheduledHandler_) {
      unscheduledHandler_->tryToFill(moduleLabel, *const_cast<EventPrincipal *>(this));
    }
    moduleLabelsRunning_.pop_back();
    return true;
  }

}
