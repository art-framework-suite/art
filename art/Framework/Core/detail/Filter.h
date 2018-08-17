#ifndef art_Framework_Core_detail_Filter_h
#define art_Framework_Core_detail_Filter_h
// vim: set sw=2 expandtab :

//==================================================================
// The base class of all modules used to control the flow of
// processing in a trigger path.  Filters can also insert products
// into the event.  These products should be informational products
// about the filter decision.
//==================================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/Modifier.h"
#include "art/Framework/Core/ProcessingFrame.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Utilities/ScheduleID.h"

#include <cstddef>

namespace art::detail {
  class Filter : public Modifier {
  public:
    static constexpr bool Pass{true};
    static constexpr bool Fail{false};

    template <typename UserConfig>
    using Table = Modifier::Table<UserConfig>;

    virtual ~Filter() noexcept;
    Filter();
    explicit Filter(fhicl::ParameterSet const&);
    Filter(Filter const&) = delete;
    Filter(Filter&&) = delete;
    Filter& operator=(Filter const&) = delete;
    Filter& operator=(Filter&&) = delete;

    void doBeginJob();
    void doEndJob();
    void doRespondToOpenInputFile(FileBlock const& fb);
    void doRespondToCloseInputFile(FileBlock const& fb);
    void doRespondToOpenOutputFiles(FileBlock const& fb);
    void doRespondToCloseOutputFiles(FileBlock const& fb);
    bool doBeginRun(RunPrincipal& rp, ModuleContext const&);
    bool doEndRun(RunPrincipal& rp, ModuleContext const& mc);
    bool doBeginSubRun(SubRunPrincipal& srp, ModuleContext const& mc);
    bool doEndSubRun(SubRunPrincipal& srp, ModuleContext const& mc);
    bool doEvent(EventPrincipal& ep,
                 ModuleContext const& mc,
                 std::atomic<std::size_t>& counts_run,
                 std::atomic<std::size_t>& counts_passed,
                 std::atomic<std::size_t>& counts_failed);

  private:
    void failureToPutProducts(ModuleDescription const& md);
    virtual void setupQueues() = 0;
    virtual void beginJobWithFrame(ProcessingFrame const&) = 0;
    virtual void endJobWithFrame(ProcessingFrame const&) = 0;
    virtual void respondToOpenInputFileWithFrame(FileBlock const&,
                                                 ProcessingFrame const&) = 0;
    virtual void respondToCloseInputFileWithFrame(FileBlock const&,
                                                  ProcessingFrame const&) = 0;
    virtual void respondToOpenOutputFilesWithFrame(FileBlock const&,
                                                   ProcessingFrame const&) = 0;
    virtual void respondToCloseOutputFilesWithFrame(FileBlock const&,
                                                    ProcessingFrame const&) = 0;
    virtual bool beginRunWithFrame(Run&, ProcessingFrame const&) = 0;
    virtual bool endRunWithFrame(Run&, ProcessingFrame const&) = 0;
    virtual bool beginSubRunWithFrame(SubRun&, ProcessingFrame const&) = 0;
    virtual bool endSubRunWithFrame(SubRun&, ProcessingFrame const&) = 0;
    virtual bool filterWithFrame(Event&, ProcessingFrame const&) = 0;

    bool checkPutProducts_{true};
  };

} // namespace art::detail

#endif /* art_Framework_Core_detail_Filter_h */

// Local Variables:
// mode: c++
// End:
