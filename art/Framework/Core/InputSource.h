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
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "cetlib/exempt_ptr.h"

#include <memory>
#include <ostream>

namespace art
{
  class MasterProductRegistry;

  namespace input {
    enum ItemType {IsInvalid, IsStop, IsFile, IsRun, IsSubRun, IsEvent};
    inline std::ostream& operator<<(std::ostream& os, ItemType const it)
    {
      switch(it) {
      case IsInvalid:
        os << "Invalid";
        break;
      case IsStop:
        os << "Stop";
        break;
      case IsFile:
        os << "InputFile";
        break;
      case IsRun:
        os << "Run";
        break;
      case IsSubRun:
        os << "SubRun";
        break;
      case IsEvent:
        os << "Event";
      }
      return os;
    }
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

    virtual ~InputSource() noexcept = default;

    // Interface expected by EventProcessor.

    // Return the Event specified by the given EventID, or the next one
    // in the input sequence after the given EventID if one with the
    // given id can not be found. Derived classes that can not perform
    // random access should not implement this function; the default
    // implementation will throw an exception.
    virtual std::unique_ptr<EventPrincipal> readEvent(EventID const& id);

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
    virtual RunID run() const = 0;
    virtual SubRunID subRun() const = 0;
    virtual std::unique_ptr<FileBlock> readFile(MasterProductRegistry&) = 0;
    virtual void closeFile() = 0;
    virtual std::unique_ptr<RunPrincipal> readRun() = 0;
    virtual std::unique_ptr<SubRunPrincipal> readSubRun(cet::exempt_ptr<RunPrincipal> rp) = 0;
    virtual std::unique_ptr<EventPrincipal> readEvent(cet::exempt_ptr<SubRunPrincipal> srp) = 0;
    virtual std::unique_ptr<RangeSetHandler> runRangeSetHandler() = 0;
    virtual std::unique_ptr<RangeSetHandler> subRunRangeSetHandler() = 0;

  };
}

#endif /* art_Framework_Core_InputSource_h */

// Local Variables:
// mode: c++
// End:
