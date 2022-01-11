#ifndef art_Framework_IO_Sources_SourceHelper_h
#define art_Framework_IO_Sources_SourceHelper_h

// -----------------------------------------------------------------
// SourceHelper provides the means for creation of EventPrincipals,
// SubRunPrincipals, and RunPrincipals.
//
// Note that processing history can only be retained for event
// principles, and only if a non-null History object is provided when
// calling makeEventPrincipal.
// -----------------------------------------------------------------

#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Persistency/Provenance/fwd.h"
#include "cetlib/exempt_ptr.h"

#include <memory>

namespace art {
  class SourceHelper;
} // namespace art

class art::SourceHelper {
public:
  explicit SourceHelper(ModuleDescription const& md);

  // Copying/moving is disallowed--the helper object that art creates
  // is intended to be passed to the user by reference for their use.
  // If the detail class uses a copy of the SourceHelper object
  // instead of a reference to the art-provided object, then the
  // 'setPresentProducts' function will not be called on the detail
  // class's object.  This will induce an exception throw when calling
  // 'make*Principal'.
  SourceHelper(SourceHelper const&) = delete;
  SourceHelper(SourceHelper&&) = delete;
  SourceHelper& operator=(SourceHelper const&) = delete;
  SourceHelper& operator=(SourceHelper&&) = delete;

  template <typename T>
  Ptr<T> makePtr(TypeLabel const& t,
                 Principal const& p,
                 typename Ptr<T>::key_type key) const;

  RunPrincipal* makeRunPrincipal(RunAuxiliary runAux) const;
  RunPrincipal* makeRunPrincipal(RunID r, Timestamp const& startTime) const;
  RunPrincipal* makeRunPrincipal(RunNumber_t r,
                                 Timestamp const& startTime) const;

  SubRunPrincipal* makeSubRunPrincipal(SubRunAuxiliary subRunAux) const;
  SubRunPrincipal* makeSubRunPrincipal(SubRunID const& sr,
                                       Timestamp const& startTime) const;
  SubRunPrincipal* makeSubRunPrincipal(RunNumber_t r,
                                       SubRunNumber_t sr,
                                       Timestamp const& startTime) const;

  EventPrincipal* makeEventPrincipal(EventAuxiliary eventAux) const;
  EventPrincipal* makeEventPrincipal(
    EventID const& e,
    Timestamp const& startTime,
    bool isRealData = true,
    EventAuxiliary::ExperimentType eType = EventAuxiliary::Data) const;
  EventPrincipal* makeEventPrincipal(
    RunNumber_t r,
    SubRunNumber_t sr,
    EventNumber_t e,
    Timestamp const& startTime,
    bool isRealData = true,
    EventAuxiliary::ExperimentType eType = EventAuxiliary::Data) const;

private:
  template <typename T>
  friend class Source;
  void throwIfProductsNotRegistered_() const;
  ProcessHistoryID processHistoryID_(BranchType,
                                     ProcessConfiguration const&) const;
  void setPresentProducts(cet::exempt_ptr<ProductTables const> presentProducts);
  cet::exempt_ptr<ProductTables const> presentProducts_{nullptr};
  ModuleDescription md_;
};

template <typename T>
art::Ptr<T>
art::SourceHelper::makePtr(TypeLabel const& tl,
                           Principal const& p,
                           typename Ptr<T>::key_type key) const
{
  BranchDescription const pd{p.branchType(),
                             tl,
                             md_.moduleLabel(),
                             md_.parameterSetID(),
                             md_.processConfiguration()};
  ProductID const pid{pd.productID()};
  return Ptr<T>{pid, key, p.productGetter(pid)};
}

#endif /* art_Framework_IO_Sources_SourceHelper_h */

// Local Variables:
// mode: c++
// End:
