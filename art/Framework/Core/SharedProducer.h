#ifndef art_Framework_Core_SharedProducer_h
#define art_Framework_Core_SharedProducer_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Producer.h"
#include "art/Framework/Core/detail/SharedModule.h"
#include "art/Framework/Principal/fwd.h"

#include <string>

namespace art {

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

#endif /* art_Framework_Core_SharedProducer_h */

// Local Variables:
// mode: c++
// End:
