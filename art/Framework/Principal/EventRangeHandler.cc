#include "art/Framework/Principal/EventRangeHandler.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Utilities/Exception.h"

#include <algorithm>
#include <iostream>

namespace {
  constexpr auto invalid_eid = art::IDNumber<art::Level::Event>::invalid();
}

namespace art {

  EventRangeHandler::EventRangeHandler()
    : EventRangeHandler{RangeSet::invalid()}
  {}

  EventRangeHandler::EventRangeHandler(RunNumber_t const r)
    : EventRangeHandler{RangeSet::forRun(r)}
  {}

  EventRangeHandler::EventRangeHandler(RangeSet const& inputRangeSet)
    : inputRanges_{inputRangeSet}
    , outputRanges_{inputRanges_}
  {}

  void
  EventRangeHandler::initializeRanges(RangeSet const& rs)
  {
    if (!inputRanges_.is_valid()) {
      inputRanges_ = rs;
      outputRanges_ = rs;
      rsIter_ = outputRanges_.ranges().begin();
    }
  }

  void
  EventRangeHandler::update(EventID const& eid, bool const lastEventOfSubRun)
  {
    lastEventOfSubRunSeen_ = lastEventOfSubRun;
    auto updateWithEmptyInput = [this](EventID const& id){
      if (outputRanges_.empty()) {
        outputRanges_.set_run(id.run());
        outputRanges_.emplace_range(id.subRun(), id.event(), id.next().event());
        rsIter_ = outputRanges_.ranges().end();
        return;
      }
      auto& back = outputRanges_.back();
      if (back.subrun() == id.subRun()) {
        if (back.end() == id.event()) {
          back.set_end(id.next().event());
        }
      }
      else {
        outputRanges_.emplace_range(id.subRun(), id.event(), id.next().event());
        rsIter_ = outputRanges_.ranges().end();
      }
    };

    auto updateWithInheritedInput = [this](EventID const& id){
      if (lastEventOfSubRunSeen_) {
        rsIter_ = end();
        return;
      }

      while (rsIter_ != end() && !rsIter_->contains(id.subRun(), id.event())) {
        ++rsIter_;
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
  EventRangeHandler::update(SubRunID const& id)
  {
    if (outputRanges_.empty()) {
      outputRanges_.set_run(id.run());
      outputRanges_.emplace_range(id.subRun(), invalid_eid, invalid_eid);
      rsIter_ = outputRanges_.ranges().begin();
    }
    else if (outputRanges_.back().subrun() != id.subRun()) {
      outputRanges_.emplace_range(id.subRun(), invalid_eid, invalid_eid);
      rsIter_ = outputRanges_.ranges().begin();
    }
  }

  void
  EventRangeHandler::setOutputRanges(RangeSet const& rs)
  {
    outputRanges_ = rs;
  }

  void
  EventRangeHandler::setOutputRanges(RangeSet::const_iterator const b,
                                     RangeSet::const_iterator const e)
  {
    outputRanges_.assign_ranges(b, e);
  }

  void
  EventRangeHandler::rebase()
  {
    if (outputRanges_.empty())
      return;

    auto const back = outputRanges_.back();
    outputRanges_.clear();
    rsIter_ = outputRanges_.ranges().end();

    if (lastEventOfSubRunSeen_) return;

    if (is_valid(back.subrun()) &&
        is_valid(back.begin()) &&
        is_valid(back.end())) {
      outputRanges_.emplace_range(back.subrun(), back.end(), IDNumber<Level::Event>::next(back.end()));
      rsIter_ = outputRanges_.ranges().end();
    }

  }

  void
  EventRangeHandler::reset()
  {
    EventRangeHandler tmp {IDNumber<Level::Run>::invalid()};
    std::swap(*this, tmp);
  }

}
