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

  // It only makes sense to track parents when putting a product onto
  // the event.  That requires a non-const Event object.
  constexpr bool record_parents(Event*) { return true; }
  constexpr bool record_parents(Event const*) { return false; }

  namespace {
    SubRun* newSubRun(EventPrincipal const& ep, ModuleDescription const& md)
    {
      return ep.subRunPrincipalExemptPtr() ? new SubRun{ep.subRunPrincipal(), md} : nullptr;
    }
  }

  Event::Event(EventPrincipal const& ep, ModuleDescription const& md)
    : DataViewImpl{ep, md, InEvent, record_parents(this)}
    , aux_{ep.aux()}
    , subRun_{newSubRun(ep, md)}
    , eventPrincipal_{ep}
    , recordParents_{record_parents(this)}
  { }

  EDProductGetter const*
  Event::productGetter(ProductID const& pid) const {
    return eventPrincipal_.productGetter(pid);
  }

  ProductID
  Event::makeProductID(BranchDescription const& desc) const {
    return ProductID{desc.branchID().id()};
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

    vector<BranchID> gotBranchIDVector;
    auto const& gotBranchIDs = retrievedProducts();
    if (!gotBranchIDs.empty()) {
      gotBranchIDVector.reserve(gotBranchIDs.size());
      gotBranchIDVector.assign(gotBranchIDs.begin(), gotBranchIDs.end());
    }

    for (auto& elem : putProducts()) {
      auto const& bd = elem.second.bd;
      auto productProvenancePtr = make_unique<ProductProvenance const>(bd.branchID(),
                                                                       productstatus::present(),
                                                                       gotBranchIDVector);
      ep.put(std::move(elem.second.prod),
             bd,
             std::move(productProvenancePtr));
    };

    // the cleanup is all or none
    putProducts().clear();
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
