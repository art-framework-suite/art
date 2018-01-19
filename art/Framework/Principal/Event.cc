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

    [[gnu::unused]]
    constexpr bool
    record_parents(Event const*)
    {
      return false;
    }

  } // unnamed namespace

  Event::~Event() = default;

  Event::Event(EventPrincipal const& ep,
               ModuleDescription const& md,
               TypeLabelLookup_t const& expectedProducts)
    : DataViewImpl{InEvent,
                   ep,
                   md,
                   record_parents(this),
                   expectedProducts,
                   RangeSet::invalid()}
    , subRun_{ep.subRunPrincipalExemptPtr() ?
                new SubRun{ep.subRunPrincipal(), md, TypeLabelLookup_t{}} :
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
