#ifndef art_Framework_Core_InputSource_h
#define art_Framework_Core_InputSource_h
// vim: set sw=2 expandtab :

//
// InputSource is the abstract interface implemented by all concrete
// sources.
//

#include "art/Framework/Core/fwd.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "cetlib/exempt_ptr.h"

#include <memory>
#include <ostream>

namespace art {

  namespace input {

    enum ItemType {
      IsInvalid, // 0
      IsStop,    // 1
      IsFile,    // 2
      IsRun,     // 3
      IsSubRun,  // 4
      IsEvent    // 5
    };

    inline std::ostream&
    operator<<(std::ostream& os, ItemType const it)
    {
      switch (it) {
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
        break;
      }
      return os;
    }

  } // namespace input

  class InputSource {
  public:
    enum ProcessingMode {
      Runs,                // 0
      RunsAndSubRuns,      // 1
      RunsSubRunsAndEvents // 2
    };

    virtual ~InputSource() noexcept;

    explicit InputSource(ModuleDescription const&);

    InputSource(InputSource const&) = delete;
    InputSource(InputSource&&) = delete;

    InputSource& operator=(InputSource const&) = delete;
    InputSource& operator=(InputSource&&) = delete;

    // Serial Access Interface
    virtual input::ItemType nextItemType() = 0;
    virtual std::unique_ptr<FileBlock> readFile() = 0;
    virtual void closeFile() = 0;
    virtual std::unique_ptr<RunPrincipal> readRun() = 0;
    virtual std::unique_ptr<SubRunPrincipal> readSubRun(
      cet::exempt_ptr<RunPrincipal const> rp) = 0;
    virtual std::unique_ptr<EventPrincipal> readEvent(
      cet::exempt_ptr<SubRunPrincipal const> srp) = 0;
    virtual std::unique_ptr<RangeSetHandler> runRangeSetHandler() = 0;
    virtual std::unique_ptr<RangeSetHandler> subRunRangeSetHandler() = 0;

    // Job Interface
    virtual void doBeginJob();
    virtual void doEndJob();

    // Random Access Interface

    // Skip forward (or backward, if n<0) n events. Derived classes
    // that cannot perform random access should not implement this
    // function; the default implementation will throw an exception.
    virtual void skipEvents(int n);

    ModuleDescription const& moduleDescription() const;
    ProcessConfiguration const& processConfiguration() const;

  private:
    ModuleDescription moduleDescription_;
  };

} // namespace art

#endif /* art_Framework_Core_InputSource_h */

// Local Variables:
// mode: c++
// End:
