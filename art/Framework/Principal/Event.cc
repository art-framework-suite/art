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
    // It only makes sense to track parents when putting a product
    // onto the event.  That requires a non-const Event object.
    bool
    should_record_parents(EventPrincipal const& ep, Event*)
    {
      if (ep.parentageEnabled()) {
        return true;
      }
      return false;
    }
    [[gnu::unused]] constexpr bool
    should_record_parents(EventPrincipal const& /*ep*/, Event const*)
    {
      return false;
    }

    SubRun*
    newSubRun(EventPrincipal const& ep,
              ModuleDescription const& md,
              cet::exempt_ptr<Consumer> consumer)
    {
      return ep.subRunPrincipalExemptPtr() ?
               new SubRun{ep.subRunPrincipal(), md, consumer} :
               nullptr;
    }
  }

  Event::Event(EventPrincipal const& ep,
               ModuleDescription const& md,
               cet::exempt_ptr<Consumer> consumer,
               RangeSet const&)
    : Event{ep, md, consumer}
  {}

  Event::Event(EventPrincipal const& ep,
               ModuleDescription const& md,
               cet::exempt_ptr<Consumer> consumer)
    : DataViewImpl{ep, md, InEvent, should_record_parents(ep, this), consumer}
    , aux_{ep.aux()}
    , subRun_{newSubRun(ep, md, consumer)}
    , eventPrincipal_{ep}
  {}

  EDProductGetter const*
  Event::productGetter(ProductID const pid) const
  {
    return eventPrincipal_.productGetter(pid);
  }

  SubRun const&
  Event::getSubRun() const
  {
    if (!subRun_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL subRun.\n";
    }
    return *subRun_;
  }

  Run const&
  Event::getRun() const
  {
    return getSubRun().getRun();
  }

  History const&
  Event::history() const
  {
    return eventPrincipal_.history();
  }

  ProcessHistoryID const&
  Event::processHistoryID() const
  {
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
    bool const process_found{
      ph.getConfigurationForProcess(processName, config)};
    if (process_found) {
      ParameterSetRegistry::get(config.parameterSetID(), ps);
    }
    return process_found;
  }

  void
  Event::commit(EventPrincipal& ep,
                bool const checkProducts,
                std::set<TypeLabel> const& expectedProducts)
  {
    assert(&ep == &eventPrincipal_);
    checkPutProducts(checkProducts, expectedProducts, putProducts());
    if (ep.parentageEnabled()) {
      auto const& parents = retrievedProductIDs();
      for (auto& elem : putProducts()) {
        auto const& pd = elem.second.pd;
        auto productProvenancePtr = make_unique<ProductProvenance const>(
          pd.productID(), productstatus::present(), parents);
        ep.put(
          std::move(elem.second.prod), pd, std::move(productProvenancePtr));
      }
    } else {
      for (auto& elem : putProducts()) {
        auto const& pd = elem.second.pd;
        auto productProvenancePtr = make_unique<ProductProvenance const>(
          pd.productID(), productstatus::present());
        ep.put(
          std::move(elem.second.prod), pd, std::move(productProvenancePtr));
      }
    }
    putProducts().clear();
  }

} // art
