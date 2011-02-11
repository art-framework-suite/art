#ifndef art_Framework_IO_Sources_RawInputSource_h
#define art_Framework_IO_Sources_RawInputSource_h

// ======================================================================
//
// RawInputSource is an abstract base class, intended to make it
// easier to write InputSources that read non-art-format data files.
//
// A concrete InputSource that inherits from RawInputSource must
// implement the pure virtual function readOneEvent().
//
// ======================================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Persistency/Provenance/EventID.h"
#include "boost/shared_ptr.hpp"
#include "fhiclcpp/ParameterSet.h"
#include <memory>

// ----------------------------------------------------------------------

namespace art 
{
  class Timestamp;

  class RawInputSource : public InputSource 
  {
  public:
    explicit RawInputSource(fhicl::ParameterSet const& pset,
                            InputSourceDescription const& desc);
    virtual ~RawInputSource();

  protected:

//     std::auto_ptr<Event> makeEvent(RunNumber_t run,
// 				   SubRunNumber_t subRun,
// 				   EventNumber_t event,
// 				   Timestamp const& tstamp);
//     virtual std::auto_ptr<Event> readOneEvent() = 0;

  private:
    virtual std::auto_ptr<EventPrincipal>      readEvent_();
    virtual boost::shared_ptr<SubRunPrincipal> readSubRun_();
    virtual boost::shared_ptr<RunPrincipal>    readRun_();
    virtual std::auto_ptr<EventPrincipal>      readIt(EventID const& eventID);
    virtual void                               skip(int offset);
    virtual input::ItemType                    getNextItemType();

    RunNumber_t                   runNumber_;
    SubRunNumber_t                subRunNumber_;
    bool                          newRun_;
    bool                          newSubRun_;
    std::auto_ptr<EventPrincipal> ep_;
  };

}  // art

// ======================================================================

#endif /* art_Framework_IO_Sources_RawInputSource_h */

// Local Variables:
// mode: c++
// End:
