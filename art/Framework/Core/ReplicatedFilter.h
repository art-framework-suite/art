#ifndef art_Framework_Core_ReplicatedFilter_h
#define art_Framework_Core_ReplicatedFilter_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/Services.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/EngineCreator.h"
#include "art/Framework/Core/detail/Filter.h"
#include "art/Framework/Principal/fwd.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace art {

  class ReplicatedFilter : public detail::Filter,
                           private detail::EngineCreator {
  public:
    using ModuleType = ReplicatedFilter;
    using WorkerType = WorkerT<ReplicatedFilter>;

    std::string workerType() const;

    explicit ReplicatedFilter(fhicl::ParameterSet const& pset,
                              Services const& services)
      : detail::EngineCreator{pset.get<std::string>("module_label"),
                              services.scheduleID()}
    {}

    template <typename Config>
    explicit ReplicatedFilter(Table<Config> const& config,
                              Services const& services)
      : ReplicatedFilter{config.get_PSet(), services}
    {}

    using detail::EngineCreator::createEngine;

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
    bool beginRunWithServices(Run&, Services const&) override final;
    bool endRunWithServices(Run&, Services const&) override final;
    bool beginSubRunWithServices(SubRun&, Services const&) override final;
    bool endSubRunWithServices(SubRun&, Services const&) override final;
    bool filterWithServices(Event&, Services const&) override final;

    virtual void beginJob(Services const&);
    virtual void endJob(Services const&);
    virtual void respondToOpenInputFile(FileBlock const&, Services const&);
    virtual void respondToCloseInputFile(FileBlock const&, Services const&);
    virtual void respondToOpenOutputFiles(FileBlock const&, Services const&);
    virtual void respondToCloseOutputFiles(FileBlock const&, Services const&);
    virtual void beginRun(Run const&, Services const&);
    virtual void endRun(Run const&, Services const&);
    virtual void beginSubRun(SubRun const&, Services const&);
    virtual void endSubRun(SubRun const&, Services const&);
    virtual bool filter(Event&, Services const&) = 0;
  };

} // namespace art

#endif /* art_Framework_Core_ReplicatedFilter_h */

// Local Variables:
// mode: c++
// End:
