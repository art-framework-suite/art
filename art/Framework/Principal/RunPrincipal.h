#ifndef art_Framework_Principal_RunPrincipal_h
#define art_Framework_Principal_RunPrincipal_h
// vim: set sw=2:

//
//  RunPrincipal
//
//  Manages per-run data products.
//
//  This is not visible to modules, instead they use the Run class,
//  which is a proxy for this class.
//

#include "art/Framework/Principal/EventRangeHandler.h"
#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Provenance/BranchMapper.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "cetlib/exempt_ptr.h"

#include <memory>
#include <vector>

namespace art {

class RunPrincipal final : public Principal {

public:

  typedef RunAuxiliary Auxiliary;

public:

  RunPrincipal(RunAuxiliary const&,
               ProcessConfiguration const&,
               EventRangeHandler const& = EventRangeHandler{ IDNumber<Level::Run>::invalid() },
               std::unique_ptr<BranchMapper>&& = std::make_unique<BranchMapper>(),
               std::unique_ptr<DelayedReader>&& = std::make_unique<NoDelayedReader>(),
               int idx = 0,
               RunPrincipal* = nullptr);

  RunAuxiliary const&
  aux() const
  {
    return aux_;
  }

  RunNumber_t
  run() const
  {
    return aux().run();
  }

  RunID const&
  id() const
  {
    return aux().id();
  }

  Timestamp const&
  beginTime() const
  {
    return aux().beginTime();
  }

  Timestamp const&
  endTime() const
  {
    return aux().endTime();
  }

  void
  setEndTime(Timestamp const& time)
  {
    aux_.setEndTime(time);
  }

  BranchType branchType() const override;

  void addGroup(BranchDescription const&);

  void addGroup(std::unique_ptr<EDProduct>&&, BranchDescription const&);

  void put(std::unique_ptr<EDProduct>&&, BranchDescription const&,
           std::unique_ptr<ProductProvenance const>&&);

  void setOutputEventRanges(RangeSet const&);
  RangeSet const& inputEventRanges() const { return rangeSetHandler_.inputRanges(); }
  RangeSet const& outputEventRanges() const { return rangeSetHandler_.outputRanges(); }

private:

  void addOrReplaceGroup(std::unique_ptr<Group>&&) override;

  ProcessHistoryID const& processHistoryID() const override;

  void setProcessHistoryID(ProcessHistoryID const&) override;

private:

  RunAuxiliary aux_;
  EventRangeHandler rangeSetHandler_;

};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_Principal_RunPrincipal_h */
