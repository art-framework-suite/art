#ifndef art_Framework_Core_InputSource_h
#define art_Framework_Core_InputSource_h

//----------------------------------------------------------------------
//
// InputSource is the abstract interface implemented by all concrete
// sources.
//
//----------------------------------------------------------------------

#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "cpp0x/memory"

namespace art
{
  class MasterProductRegistry;

  namespace input
  {
    enum ItemType
      {
        IsInvalid,
        IsStop,
        IsFile,
        IsRun,
        IsSubRun,
        IsEvent
      };
  }

  class InputSource
  {
  public:

    // TODO:
    // This enum should probably be moved outside of InputSource.
    enum ProcessingMode
      {
        Runs,
        RunsAndSubRuns,
        RunsSubRunsAndEvents
      };

    virtual ~InputSource();

    // Interface expected by EventProcessor.

    // Return the Event specified by the given EventID, or the next one
    // in the input sequence after the given EventID if one with the
    // given id can not be found. Derived classes that can not perform
    // random access should not implement this function; the default
    // implementation will throw an exception.
    virtual std::auto_ptr<EventPrincipal> readEvent(EventID const& id);

    // Skip forward (or backward, if n<0) n events. Derived classes that
    // can not perform random access should not implement this function;
    // the default implementation will throw an exception.
    virtual void skipEvents(int n);

    // Rewind to the beginning of input. Derived classes that can not
    // perform this function will throw an exception.
    virtual void rewind();

    virtual void doBeginJob();
    virtual void doEndJob();

    virtual input::ItemType nextItemType() = 0;
    virtual RunNumber_t run() const = 0;
    virtual SubRunNumber_t subRun() const = 0;
    virtual std::shared_ptr<FileBlock> readFile(MasterProductRegistry&) = 0;
    virtual void closeFile() = 0;
    virtual std::shared_ptr<RunPrincipal> readRun() = 0;
    virtual std::shared_ptr<SubRunPrincipal> readSubRun(std::shared_ptr<RunPrincipal> rp) = 0;
    virtual std::auto_ptr<EventPrincipal> readEvent(std::shared_ptr<SubRunPrincipal> srp) = 0;
  };
}

#endif /* art_Framework_Core_InputSource_h */

// Local Variables:
// mode: c++
// End:
