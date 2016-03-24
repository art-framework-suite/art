#include "art/Framework/Principal/EventRangeHandler.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Utilities/Exception.h"

#include <algorithm>

namespace art {

  EventRangeHandler::EventRangeHandler(RunNumber_t const r)
    : EventRangeHandler{RangeSet{r}}
  {}

  EventRangeHandler::EventRangeHandler(RangeSet const& inputRangeSet)
    : inputRanges_{inputRangeSet}
    , outputRanges_{inputRanges_}
  {}

  void
  EventRangeHandler::update(EventID const& eid, bool const lastEventOfSubRun)
  {
    lastEventOfSubRunSeen_ = lastEventOfSubRun;
    auto updateWithEmptyInput = [this](EventID const& id){
      if (outputRanges_.empty()) {
        outputRanges_.set_and_emplace_range(id.run(),
                                            id.subRun(), id.event(), id.next().event());
        return;
      }
      auto& back = outputRanges_.back();
      if ( back.subrun() == id.subRun() ) {
        if ( back.end() == id.event() ) {
          back.set_end(id.next().event());
        }
      }
      else {
        outputRanges_.emplace_range(id.subRun(), id.event(), id.next().event());
      }
    };

    auto updateWithInheritedInput = [this](EventID const& id){
      while (!inputRanges_.empty() && !inputRanges_.front().contains(id.event())) {
        inputRanges_.pop_front();
        auto const& front = inputRanges_.front();
        outputRanges_.emplace_range(front.subrun(),
                                    front.begin(),
                                    front.begin());
      }
      if (lastEventOfSubRunSeen_) {
        outputRanges_.back().set_end(inputRanges_.front().end());
      }
      else {
        outputRanges_.back().set_end(id.next().event());
      }
    };

    if (inputRanges_.empty()) {
      updateWithEmptyInput(eid);
    }
    else {
      updateWithInheritedInput(eid);
    }
  }

  void
  EventRangeHandler::setOutputRanges(RangeSet const& rs)
  {
    outputRanges_ = rs;
  }

  void
  EventRangeHandler::rebase()
  {
    if (outputRanges_.empty())
      return;

    auto const back = outputRanges_.back();
    outputRanges_.clear();

    if (lastEventOfSubRunSeen_) return;

    if (is_valid(back.subrun()) &&
        is_valid(back.begin()) &&
        is_valid(back.end())) {
      outputRanges_.emplace_range(back.subrun(), back.end(), IDNumber<Level::Event>::next(back.end()));
    }

  }

}