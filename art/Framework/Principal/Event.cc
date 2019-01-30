#include "art/Framework/Principal/Event.h"
// vim: set sw=2 expandtab :

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

    // It only makes sense to track parents when putting a product onto
    // the event.  That requires a non-const Event object.
    constexpr bool
    record_parents(Event*)
    {
      return true;
    }

  } // unnamed namespace

  Event::~Event() = default;

  Event::Event(EventPrincipal const& ep, ModuleContext const& mc)
    : DataViewImpl{InEvent, ep, mc, record_parents(this), RangeSet::invalid()}
    , subRun_{ep.subRunPrincipalPtr() ? new SubRun{ep.subRunPrincipal(), mc} :
                                        nullptr}
  {}

  EventID
  Event::id() const
  {
    return DataViewImpl::eventID();
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

} // namespace art
