/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include "FWCore/Integration/test/TestRunSubRunSource.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/ParameterSet/ParameterSet.h"
#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Persistency/Provenance/Timestamp.h"
#include "art/Framework/Core/InputSourceMacros.h"

namespace art {

  TestRunSubRunSource::TestRunSubRunSource(ParameterSet const& pset,
				       InputSourceDescription const& desc) :
    InputSource(pset, desc),
    runSubRunEvent_(pset.getUntrackedParameter<std::vector<int> >("runSubRunEvent", std::vector<int>())),
    currentIndex_(0),
    firstTime_(true) {
  }

  TestRunSubRunSource::~TestRunSubRunSource() {
  }

  boost::shared_ptr<RunPrincipal>
  TestRunSubRunSource::readRun_() {
    unsigned int run = runSubRunEvent_[currentIndex_];
    Timestamp ts = Timestamp(1);  // 1 is just a meaningless number to make it compile for the test

    RunAuxiliary runAux(run, ts, Timestamp::invalidTimestamp());
    boost::shared_ptr<RunPrincipal> runPrincipal(
        new RunPrincipal(runAux, productRegistry(), processConfiguration()));
    currentIndex_ += 3;
    return runPrincipal;
  }

  boost::shared_ptr<SubRunPrincipal>
  TestRunSubRunSource::readSubRun_() {
    unsigned int run = runSubRunEvent_[currentIndex_];
    unsigned int subRun = runSubRunEvent_[currentIndex_ + 1];
    Timestamp ts = Timestamp(1);

    RunAuxiliary runAux(run, ts, Timestamp::invalidTimestamp());
    boost::shared_ptr<RunPrincipal> rp2(
        new RunPrincipal(runAux, productRegistry(), processConfiguration()));

    SubRunAuxiliary subRunAux(rp2->run(), subRun, ts, Timestamp::invalidTimestamp());
    boost::shared_ptr<SubRunPrincipal> subRunPrincipal(
        new SubRunPrincipal(subRunAux, productRegistry(), processConfiguration()));
    subRunPrincipal->setRunPrincipal(rp2);

    currentIndex_ += 3;
    return subRunPrincipal;
  }

  std::auto_ptr<EventPrincipal>
  TestRunSubRunSource::readEvent_() {
    EventSourceSentry(*this);
    unsigned int run = runSubRunEvent_[currentIndex_];
    unsigned int subRun = runSubRunEvent_[currentIndex_ + 1];
    unsigned int event = runSubRunEvent_[currentIndex_ + 2];
    Timestamp ts = Timestamp(1);

    RunAuxiliary runAux(run, ts, Timestamp::invalidTimestamp());
    boost::shared_ptr<RunPrincipal> rp2(
        new RunPrincipal(runAux, productRegistry(), processConfiguration()));

    SubRunAuxiliary subRunAux(rp2->run(), subRun, ts, Timestamp::invalidTimestamp());
    boost::shared_ptr<SubRunPrincipal> lbp2(
        new SubRunPrincipal(subRunAux, productRegistry(), processConfiguration()));
    lbp2->setRunPrincipal(rp2);

    EventID id(run, event);
    currentIndex_ += 3;
    EventAuxiliary eventAux(id, processGUID(), ts, lbp2->subRun(), false);
    std::auto_ptr<EventPrincipal> result(
	new EventPrincipal(eventAux, productRegistry(), processConfiguration()));
    result->setSubRunPrincipal(lbp2);
    return result;
  }

  InputSource::ItemType
  TestRunSubRunSource::getNextItemType() {
    if (firstTime_) {
      firstTime_ = false;
      return InputSource::IsFile;
    }
    if (currentIndex_ + 2 >= runSubRunEvent_.size()) {
      return InputSource::IsStop;
    }
    if (runSubRunEvent_[currentIndex_] == 0) {
      return InputSource::IsStop;
    }
    ItemType oldState = state();
    if (oldState == IsInvalid) return InputSource::IsFile;
    if (runSubRunEvent_[currentIndex_ + 1] == 0) {
      return InputSource::IsRun;
    }
    if (runSubRunEvent_[currentIndex_ + 2] == 0) {
      return InputSource::IsSubRun;
    }
    return InputSource::IsEvent;
  }
}

using art::TestRunSubRunSource;
DEFINE_ART_INPUT_SOURCE(TestRunSubRunSource);

