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
    virtual void beginRun(Run&, Services const&);
    virtual void endRun(Run&, Services const&);
    virtual void beginSubRun(SubRun&, Services const&);
    virtual void endSubRun(SubRun&, Services const&);
    virtual bool filter(Event&, Services const&) = 0;
  };

} // namespace art

#endif /* art_Framework_Core_SharedFilter_h */

// Local Variables:
// mode: c++
// End:
