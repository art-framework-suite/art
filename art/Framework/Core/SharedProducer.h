#ifndef art_Framework_Core_SharedProducer_h
#define art_Framework_Core_SharedProducer_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Producer.h"
#include "art/Framework/Core/detail/SharedModule.h"
#include "art/Framework/Core/fwd.h"
#include "art/Framework/Principal/fwd.h"

#include <string>

namespace art {
  namespace detail {
    class SharedResources;
  }

  class SharedProducer : public detail::Producer, public detail::SharedModule {
  public:
    using ModuleType = SharedProducer;
    using WorkerType = WorkerT<SharedProducer>;

    explicit SharedProducer(fhicl::ParameterSet const& pset)
      : detail::Producer{pset}
      , detail::SharedModule{pset.get<std::string>("module_label", {})}
    {}

    template <typename Config>
    explicit SharedProducer(Table<Config> const& config)
      : SharedProducer{config.get_PSet()}
    {}

  private:
    void setupQueues(detail::SharedResources const&) override final;
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
    void beginRunWithFrame(Run&, ProcessingFrame const&) override final;
    void endRunWithFrame(Run&, ProcessingFrame const&) override final;
    void beginSubRunWithFrame(SubRun&, ProcessingFrame const&) override final;
    void endSubRunWithFrame(SubRun&, ProcessingFrame const&) override final;
    void produceWithFrame(Event&, ProcessingFrame const&) override final;

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
    virtual void produce(Event&, ProcessingFrame const&) = 0;
  };

} // namespace art

#endif /* art_Framework_Core_SharedProducer_h */

// Local Variables:
// mode: c++
// End:
