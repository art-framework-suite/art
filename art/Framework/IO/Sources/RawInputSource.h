#ifndef FWCore_Sources_RawInputSource_h
#define FWCore_Sources_RawInputSource_h

/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include <memory>
#include "boost/shared_ptr.hpp"

#include "art/Persistency/Provenance/EventID.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/Frameworkfwd.h"

namespace edm {
  class ParameterSet;
  class Timestamp;
  class RawInputSource : public InputSource {
  public:
    explicit RawInputSource(ParameterSet const& pset, InputSourceDescription const& desc);
    virtual ~RawInputSource();

  protected:
    std::auto_ptr<Event> makeEvent(RunNumber_t run, SubRunNumber_t lumi, EventNumber_t event, Timestamp const& tstamp);
    virtual std::auto_ptr<Event> readOneEvent() = 0;

  private:
    virtual std::auto_ptr<EventPrincipal> readEvent_();
    virtual boost::shared_ptr<SubRunPrincipal> readLuminosityBlock_();
    virtual boost::shared_ptr<RunPrincipal> readRun_();
    virtual std::auto_ptr<EventPrincipal> readIt(EventID const& eventID);
    virtual void skip(int offset);
    virtual ItemType getNextItemType();

    RunNumber_t runNumber_;
    SubRunNumber_t luminosityBlockNumber_;
    bool newRun_;
    bool newLumi_;
    std::auto_ptr<EventPrincipal> ep_;
  };
}
#endif
