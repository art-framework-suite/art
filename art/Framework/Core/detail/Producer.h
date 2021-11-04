#ifndef art_Framework_Core_detail_Producer_h
#define art_Framework_Core_detail_Producer_h
// vim: set sw=2 expandtab :

//=====================================================
// The base class of modules whose main purpose is to
// insert products into an Event.
//=====================================================

#include "art/Framework/Core/Modifier.h"
#include "art/Framework/Core/ProcessingFrame.h"
#include "art/Framework/Core/fwd.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/fwd.h"
#include "art/Utilities/ScheduleID.h"

#include <cstddef>

namespace art::detail {
  class SharedResources;

  class Producer : public Modifier {
  public:
    template <typename UserConfig, typename KeysToIgnore = void>
    using Table = Modifier::Table<UserConfig, KeysToIgnore>;

    virtual ~Producer() noexcept;
    explicit Producer(fhicl::ParameterSet const&);
    Producer(Producer const&) = delete;
    Producer(Producer&&) = delete;
    Producer& operator=(Producer const&) = delete;
    Producer& operator=(Producer&&) = delete;

    // Interface provided for the worker.
    void doBeginJob(SharedResources const& resources);
    void doEndJob();
    void doRespondToOpenInputFile(FileBlock const& fb);
    void doRespondToCloseInputFile(FileBlock const& fb);
    void doRespondToOpenOutputFiles(FileBlock const& fb);
    void doRespondToCloseOutputFiles(FileBlock const& fb);
    bool doBeginRun(RunPrincipal& rp, ModuleContext const& mc);
    bool doEndRun(RunPrincipal& rp, ModuleContext const& mc);
    bool doBeginSubRun(SubRunPrincipal& srp, ModuleContext const& mc);
    bool doEndSubRun(SubRunPrincipal& srp, ModuleContext const& mc);
    bool doEvent(EventPrincipal& ep,
                 ModuleContext const& mc,
                 std::atomic<std::size_t>& counts_run,
                 std::atomic<std::size_t>& counts_passed,
                 std::atomic<std::size_t>& counts_failed);

  private:
    virtual void setupQueues(SharedResources const&) = 0;
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
    virtual void beginRunWithFrame(Run&, ProcessingFrame const&) = 0;
    virtual void endRunWithFrame(Run&, ProcessingFrame const&) = 0;
    virtual void beginSubRunWithFrame(SubRun&, ProcessingFrame const&) = 0;
    virtual void endSubRunWithFrame(SubRun&, ProcessingFrame const&) = 0;
    virtual void produceWithFrame(Event&, ProcessingFrame const&) = 0;

    bool const checkPutProducts_;
  };

} // namespace art::detail

#endif /* art_Framework_Core_detail_Producer_h */

// Local Variables:
// mode: c++
// End:
