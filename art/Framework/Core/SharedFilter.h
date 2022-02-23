#ifndef art_Framework_Core_SharedFilter_h
#define art_Framework_Core_SharedFilter_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Filter.h"
#include "art/Framework/Core/detail/SharedModule.h"
#include "art/Framework/Core/fwd.h"
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

  private:
    void setupQueues(detail::SharedResources const& resources) final;
    void beginJobWithFrame(ProcessingFrame const&) final;
    void endJobWithFrame(ProcessingFrame const&) final;
    void respondToOpenInputFileWithFrame(FileBlock const&,
                                         ProcessingFrame const&) final;
    void respondToCloseInputFileWithFrame(FileBlock const&,
                                          ProcessingFrame const&) final;
    void respondToOpenOutputFilesWithFrame(FileBlock const&,
                                           ProcessingFrame const&) final;
    void respondToCloseOutputFilesWithFrame(FileBlock const&,
                                            ProcessingFrame const&) final;
    bool beginRunWithFrame(Run&, ProcessingFrame const&) final;
    bool endRunWithFrame(Run&, ProcessingFrame const&) final;
    bool beginSubRunWithFrame(SubRun&, ProcessingFrame const&) final;
    bool endSubRunWithFrame(SubRun&, ProcessingFrame const&) final;
    bool filterWithFrame(Event&, ProcessingFrame const&) final;

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

  extern template class WorkerT<SharedFilter>;

} // namespace art

#endif /* art_Framework_Core_SharedFilter_h */

// Local Variables:
// mode: c++
// End:
