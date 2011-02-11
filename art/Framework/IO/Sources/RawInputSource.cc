/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include "art/Framework/IO/Sources/RawInputSource.h"

#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Persistency/Provenance/Timestamp.h"


namespace art {

  RawInputSource::RawInputSource(fhicl::ParameterSet const& pset,
                                 InputSourceDescription const& desc) :
    InputSource(pset, desc),
    runNumber_(RunNumber_t()),
    newRun_(false),
    newSubRun_(false),
    ep_(0) {
      setTimestamp(Timestamp::beginOfTime());
  }

  RawInputSource::~RawInputSource() 
  { }

  boost::shared_ptr<RunPrincipal>
  RawInputSource::readRun_() {
    newRun_ = false;
    RunAuxiliary runAux(RunID(runNumber_),
                        timestamp(), Timestamp::invalidTimestamp());
    return boost::shared_ptr<RunPrincipal>(
        new RunPrincipal(runAux,
                         productRegistry(),
                         processConfiguration()));
  }

  boost::shared_ptr<SubRunPrincipal>
  RawInputSource::readSubRun_() {
    newSubRun_ = false;
    SubRunAuxiliary subRunAux(SubRunID(runNumber_, subRunNumber_),
                              timestamp(), Timestamp::invalidTimestamp());
    return boost::shared_ptr<SubRunPrincipal>(
        new SubRunPrincipal(subRunAux,
                                     productRegistry(),
                                     processConfiguration()));
  }

  std::auto_ptr<EventPrincipal>
  RawInputSource::readEvent_() {
    assert(ep_.get() != 0);
    return ep_;
  }

//   std::auto_ptr<Event>
//   RawInputSource::makeEvent(RunNumber_t run, SubRunNumber_t subRun, EventNumber_t event, Timestamp const& tstamp) {
//     EventSourceSentry sentry(*this);
//     EventAuxiliary eventAux(EventID(run, subRun, event),
//       processGUID(), tstamp, true, EventAuxiliary::Data);
//     ep_ = std::auto_ptr<EventPrincipal>(
//         new EventPrincipal(eventAux, productRegistry(), processConfiguration()));
//     std::auto_ptr<Event> e(new Event(*ep_, moduleDescription()));
//     return e;
//   }


  input::ItemType
  RawInputSource::getNextItemType() {
    return input::IsStop;
  }

//     if (state() == input::IsInvalid) {
//       return input::IsFile;
//     }
//     if (newRun_) {
//       return input::IsRun;
//     }
//     if (newSubRun_) {
//       return input::IsSubRun;
//     }
//     if(ep_.get() != 0) {
//       return input::IsEvent;
//     }
//     std::auto_ptr<Event> e(readOneEvent());
//     if (e.get() == 0) {
//       return input::IsStop;
//     } else {
//        commitEvent(*e);
//     }
//     if (e->run() != runNumber_) {
//       newRun_ = newSubRun_ = true;
//       resetSubRunPrincipal();
//       resetRunPrincipal();
//       runNumber_ = e->run();
//       subRunNumber_ = e->subRun();
//       return input::IsRun;
//     } else if (e->subRun() != subRunNumber_) {
//       subRunNumber_ = e->subRun();
//       newSubRun_ = true;
//       resetSubRunPrincipal();
//       return input::IsSubRun;
//     }
//     return input::IsEvent;
//   }

  std::auto_ptr<EventPrincipal>
  RawInputSource::readIt(EventID const&) {
      throw art::Exception(errors::LogicError,"RawInputSource::readEvent_(EventID const& eventID)")
        << "Random access read cannot be used for RawInputSource.\n"
        << "Contact a Framework developer.\n";
  }

  // Not yet implemented
  void
  RawInputSource::skip(int) {
      throw art::Exception(errors::LogicError,"RawInputSource::skip(int offset)")
        << "Random access skip cannot be used for RawInputSource\n"
        << "Contact a Framework developer.\n";
  }

}  // art
