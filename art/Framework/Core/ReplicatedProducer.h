#ifndef art_Framework_Core_ReplicatedProducer_h
#define art_Framework_Core_ReplicatedProducer_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/EngineCreator.h"
#include "art/Framework/Core/detail/Producer.h"
#include "art/Framework/Principal/fwd.h"

#include <string>

namespace art {

  class ReplicatedProducer : public detail::Producer,
                             private detail::EngineCreator {
  public:
    using ModuleType = ReplicatedProducer;
    using WorkerType = WorkerT<ReplicatedProducer>;

    // Only the TriggerResults module is allowed to have no
    // module_label parameter.  We provide a default empty string for
    // only that reason.
    explicit ReplicatedProducer(fhicl::ParameterSet const& pset,
                                ScheduleID const scheduleID)
      : detail::EngineCreator{pset.get<std::string>("module_label", {}),
                              scheduleID}
    {}

    template <typename Config>
    explicit ReplicatedProducer(Table<Config> const& config,
                                ScheduleID const scheduleID)
      : ReplicatedProducer{config.get_PSet(), scheduleID}
    {}

    using detail::EngineCreator::createEngine;

    std::string workerType() const;

  private:
    void setupQueues() override final;
    void beginJobWithServices(Services const&) override final;
    void endJobWithServices(Services const&) override final;
    void respondToOpenInputFileWithServices(FileBlock const&,
                                            Services const&) override final;
    void respondToCloseInputFileWithServices(FileBlock const&,
                                             Services const&) override final;
    void respondToOpenOutputFilesWithServices(FileBlock const&,
                                              Services const&) override final;
    void respondToCloseOutputFilesWithServices(FileBlock const&,
                                               Services const&) override final;
    void beginRunWithServices(Run&, Services const&) override final;
    void endRunWithServices(Run&, Services const&) override final;
    void beginSubRunWithServices(SubRun&, Services const&) override final;
    void endSubRunWithServices(SubRun&, Services const&) override final;
    void produceWithServices(Event&, Services const&) override final;

    virtual void beginJob(Services const&);
    virtual void endJob(Services const&);
    virtual void respondToOpenInputFile(FileBlock const&, Services const&);
    virtual void respondToCloseInputFile(FileBlock const&, Services const&);
    virtual void respondToOpenOutputFiles(FileBlock const&, Services const&);
    virtual void respondToCloseOutputFiles(FileBlock const&, Services const&);
    virtual void beginRun(Run&, Services const&);
    virtual void endRun(Run&, Services const&);
    virtual void beginSubRun(SubRun&, Services const&);
    virtual void endSubRun(SubRun&, Services const&);
    virtual void produce(Event&, Services const&) = 0;
  };

} // namespace art

#endif /* art_Framework_Core_ReplicatedProducer_h */

// Local Variables:
// mode: c++
// End:
