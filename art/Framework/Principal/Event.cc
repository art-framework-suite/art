// ======================================================================
//
// Event - This is the primary interface for accessing EDProducts from a
//         single collision and inserting new derived products.
//
// ======================================================================

#include "art/Framework/Principal/Event.h"

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "fhiclcpp/ParameterSetRegistry.h"

using namespace std;
using namespace fhicl;

namespace art {

  namespace {
    SubRun* newSubRun(EventPrincipal const& ep, ModuleDescription const& md)
    {
      return ep.subRunPrincipalExemptPtr() ? new SubRun{ep.subRunPrincipal(), md} : nullptr;
    }
  }

  Event::Event(EventPrincipal const& ep, ModuleDescription const& md)
    : DataViewImpl{ep, md, InEvent}
    , aux_{ep.aux()}
    , subRun_{newSubRun(ep, md)}
    , eventPrincipal_{ep}
  { }

  EDProductGetter const*
  Event::productGetter(ProductID const& pid) const {
    return eventPrincipal_.productGetter(pid);
  }

  ProductID
  Event::branchIDToProductID(BranchID const bid) const {
    return eventPrincipal_.branchIDToProductID(bid);
  }

  ProductID
  Event::makeProductID(BranchDescription const& desc) const {
    return eventPrincipal_.branchIDToProductID(desc.branchID());
  }

  SubRun const&
  Event::getSubRun() const {
    if (!subRun_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL subRun.\n";
    }
    return *subRun_;
  }

  Run const&
  Event::getRun() const {
    return getSubRun().getRun();
  }

  History const&
  Event::history() const {
    return eventPrincipal_.history();
  }

  ProcessHistoryID const&
  Event::processHistoryID() const {
    return eventPrincipal_.history().processHistoryID();
  }


  bool
  Event::getProcessParameterSet(string const& processName,
                                ParameterSet& ps) const
  {
    // Get the ProcessHistory for this event.
    ProcessHistory ph;
    if (!ProcessHistoryRegistry::get(processHistoryID(), ph)) {
      throw Exception(errors::NotFound)
        << "ProcessHistoryID " << processHistoryID()
        << " is claimed to describe " << id()
        << "\nbut is not found in the ProcessHistoryRegistry.\n"
        "This file is malformed.\n";
    }

    ProcessConfiguration config;
    bool const process_found {ph.getConfigurationForProcess(processName, config)};
    if (process_found) {
      ParameterSetRegistry::get(config.parameterSetID(), ps);
    }
    return process_found;
  }

  GroupQueryResult
  Event::getByProductID_(ProductID const& oid) const
  {
    return eventPrincipal_.getByProductID(oid);
  }


  void
  Event::commit_(EventPrincipal& ep, bool const checkProducts, std::set<TypeLabel> const& expectedProducts)
  {
    // Check addresses only since type of 'ep' will hopefully change to Principal&.
    assert(&ep == &eventPrincipal_);
    checkPutProducts(checkProducts, expectedProducts, putProducts());
    commit_aux(ep, putProducts());
  }

  void
  Event::commit_aux(EventPrincipal& ep, Base::TypeLabelMap& products)
  {
    vector<BranchID> gotBranchIDVector;
    if (!gotBranchIDs_.empty()) {
      gotBranchIDVector.reserve(gotBranchIDs_.size());
      gotBranchIDVector.assign(gotBranchIDs_.begin(), gotBranchIDs_.end());
    }

    auto put_in_principal = [&gotBranchIDVector, &ep](auto& elem) {

      // set provenance
      auto const& bd = elem.second.bd;
      auto productProvenancePtr = make_unique<ProductProvenance const>(bd.branchID(),
                                                                       productstatus::present(),
                                                                       gotBranchIDVector);

      if (!ep.branchIDToProductID(bd.branchID()).isValid()) {
        throw art::Exception(art::errors::ProductPutFailure, "Null Product ID")
          << "put: Cannot put product with null Product ID.\n";
      }

      ep.put(std::move(elem.second.prod),
             bd,
             std::move(productProvenancePtr));
    };

    cet::for_all(products, put_in_principal);

    // the cleanup is all or none
    products.clear();
  }

  void
  Event::addToGotBranchIDs(Provenance const& prov) const {
    if (prov.branchDescription().transient()) {
      // If the product retrieved is transient, don't use its branch ID.
      // use the branch ID's of its parents.
      vector<BranchID> const& bids = prov.parents();
      gotBranchIDs_.insert(bids.begin(), bids.end());
    } else {
      gotBranchIDs_.insert(prov.branchID());
    }
  }

  // ----------------------------------------------------------------------

  void
  Event::ensure_unique_product(std::size_t const  nFound,
                               TypeID      const& typeID,
                               std::string const& moduleLabel,
                               std::string const& productInstanceName,
                               std::string const& processName) const
  {
    if (nFound == 1) return;

    art::Exception e(art::errors::ProductNotFound);
    e << "getView: Found "
      << (nFound == 0 ? "no products"
          : "more than one product"
          )
      << " matching all criteria\n"
      << "Looking for sequence of type: " << typeID << "\n"
      << "Looking for module label: " << moduleLabel << "\n"
      << "Looking for productInstanceName: " << productInstanceName << "\n";
    if (!processName.empty())
      e << "Looking for processName: "<< processName <<"\n";
    throw e;
  }

}  // art

// ======================================================================
