#ifndef art_Framework_Core_SharedFilter_h
#define art_Framework_Core_SharedFilter_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Filter.h"
#include "art/Framework/Core/detail/SharedModule.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ScheduleID.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace art {

  class SharedFilter : public detail::Filter, public detail::SharedModule {
  public:
    using ModuleType = SharedFilter;
    using WorkerType = WorkerT<SharedFilter>;

    explicit SharedFilter(fhicl::ParameterSet const& pset)
      : detail::Filter{pset}
      , detail::SharedModule{pset.get<std::string>("module_label")}
    {}

    template <typename Config>
    explicit SharedFilter(Table<Config> const& config)
      : SharedFilter{config.get_PSet()}
    {}

    std::string workerType() const;

  private:
    void setupQueues() override final;
    void beginJobWithFrame(ProcessingFrame const&) override final;
    void endJobWithFrame(ProcessingFrame const&) override final;
    void respondToOpenInputFileWithFrame(FileBlock const&,
                                         ProcessingFrame const&) override final;
    void respondToCloseInputFileWithFrame(
      FileBlock const&,
      ProcessingFrame const&) override final;
    void respondToOpenOutputFilesWithFrame(
      FileBlock const&,
      ProcessingFrame const&) override final;
    void respondToCloseOutputFilesWithFrame(
      FileBlock const&,
      ProcessingFrame const&) override final;
    bool beginRunWithFrame(Run&, ProcessingFrame const&) override final;
    bool endRunWithFrame(Run&, ProcessingFrame const&) override final;
    bool beginSubRunWithFrame(SubRun&, ProcessingFrame const&) override final;
    bool endSubRunWithFrame(SubRun&, ProcessingFrame const&) override final;
    bool filterWithFrame(Event&, ProcessingFrame const&) override final;

    virtual void beginJob(ProcessingFrame const&);
    virtual void endJob(ProcessingFrame const&);
    virtual void respondToOpenInputFile(FileBlock const&,
                                        ProcessingFrame const&);
    virtual void respondToCloseInputFile(FileBlock const&,
                                         ProcessingFrame const&);
    virtual void respondToOpenOutputFiles(FileBlock const&,
                                          ProcessingFrame const&);
    virtual void respondToCloseOutputFiles(FileBlock const&,
                                           ProcessingFrame const&);
    virtual void beginRun(Run&, ProcessingFrame const&);
    virtual void endRun(Run&, ProcessingFrame const&);
    virtual void beginSubRun(SubRun&, ProcessingFrame const&);
    virtual void endSubRun(SubRun&, ProcessingFrame const&);
    virtual bool filter(Event&, ProcessingFrame const&) = 0;
  };

} // namespace art

#endif /* art_Framework_Core_SharedFilter_h */

// Local Variables:
// mode: c++
// End:
