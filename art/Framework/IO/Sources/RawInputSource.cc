/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include "art/Framework/IO/Sources/RawInputSource.h"
#include "art/Persistency/Provenance/Timestamp.h"
#include "art/Framework/Core/EventPrincipal.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Framework/Core/Event.h"

namespace edm {
  RawInputSource::RawInputSource(ParameterSet const& pset,
				       InputSourceDescription const& desc) :
    InputSource(pset, desc),
    runNumber_(RunNumber_t()),
    newRun_(false),
    newLumi_(false),
    ep_(0) {
      setTimestamp(Timestamp::beginOfTime());
  }

  RawInputSource::~RawInputSource() {
  }

  boost::shared_ptr<RunPrincipal>
  RawInputSource::readRun_() {
    newRun_ = false;
    RunAuxiliary runAux(runNumber_, timestamp(), Timestamp::invalidTimestamp());
    return boost::shared_ptr<RunPrincipal>(
	new RunPrincipal(runAux,
			 productRegistry(),
			 processConfiguration()));
  }

  boost::shared_ptr<SubRunPrincipal>
  RawInputSource::readLuminosityBlock_() {
    newLumi_ = false;
    SubRunAuxiliary lumiAux(runNumber_,
	luminosityBlockNumber_, timestamp(), Timestamp::invalidTimestamp());
    return boost::shared_ptr<SubRunPrincipal>(
	new SubRunPrincipal(lumiAux,
				     productRegistry(),
				     processConfiguration()));
  }

  std::auto_ptr<EventPrincipal>
  RawInputSource::readEvent_() {
    assert(ep_.get() != 0);
    return ep_;
  }

  std::auto_ptr<Event>
  RawInputSource::makeEvent(RunNumber_t run, SubRunNumber_t lumi, EventNumber_t event, Timestamp const& tstamp) {
    EventSourceSentry sentry(*this);
    EventAuxiliary eventAux(EventID(run, event),
      processGUID(), tstamp, lumi, true, EventAuxiliary::Data);
    ep_ = std::auto_ptr<EventPrincipal>(
	new EventPrincipal(eventAux, productRegistry(), processConfiguration()));
    std::auto_ptr<Event> e(new Event(*ep_, moduleDescription()));
    return e;
  }


  InputSource::ItemType
  RawInputSource::getNextItemType() {
    if (state() == IsInvalid) {
      return IsFile;
    }
    if (newRun_) {
      return IsRun;
    }
    if (newLumi_) {
      return IsSubRun;
    }
    if(ep_.get() != 0) {
      return IsEvent;
    }
    std::auto_ptr<Event> e(readOneEvent());
    if (e.get() == 0) {
      return IsStop;
    } else {
      e->commit_();
    }
    if (e->run() != runNumber_) {
      newRun_ = newLumi_ = true;
      resetSubRunPrincipal();
      resetRunPrincipal();
      runNumber_ = e->run();
      luminosityBlockNumber_ = e->luminosityBlock();
      return IsRun;
    } else if (e->luminosityBlock() != luminosityBlockNumber_) {
      luminosityBlockNumber_ = e->luminosityBlock();
      newLumi_ = true;
      resetSubRunPrincipal();
      return IsSubRun;
    }
    return IsEvent;
  }

  std::auto_ptr<EventPrincipal>
  RawInputSource::readIt(EventID const&) {
      throw edm::Exception(errors::LogicError,"RawInputSource::readEvent_(EventID const& eventID)")
        << "Random access read cannot be used for RawInputSource.\n"
        << "Contact a Framework developer.\n";
  }

  // Not yet implemented
  void
  RawInputSource::skip(int) {
      throw edm::Exception(errors::LogicError,"RawInputSource::skip(int offset)")
        << "Random access skip cannot be used for RawInputSource\n"
        << "Contact a Framework developer.\n";
  }

}
