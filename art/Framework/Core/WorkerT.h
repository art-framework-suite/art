#ifndef art_Framework_Core_WorkerT_h
#define art_Framework_Core_WorkerT_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/ModuleType.h"
#include "art/Utilities/SharedResource.h"
#include "fhiclcpp/ParameterSet.h"

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <type_traits>

namespace art {
  template <typename T>
  class WorkerT : public Worker {
    // Let PathManager use module() to delete the trigger results inserter.
    friend class PathManager;

  public: // TYPES
    using ModuleType = T;

    // This is called directly by the make_worker function created by
    // the DEFINE_ART_MODULE macro.
    WorkerT(std::shared_ptr<T>, ModuleDescription const&, WorkerParams const&);

  protected: // MEMBER FUNCTIONS -- API for implementation classes
    T&
    module()
    {
      return *module_;
    }
    T const&
    module() const
    {
      return *module_;
    }

  private:
    std::string workerType() const override;
    hep::concurrency::SerialTaskQueueChain* implSerialTaskQueueChain()
      const override;
    void implBeginJob(detail::SharedResources const&) override;
    void implEndJob() override;
    void implRespondToOpenInputFile(FileBlock const&) override;
    void implRespondToCloseInputFile(FileBlock const&) override;
    void implRespondToOpenOutputFiles(FileBlock const&) override;
    void implRespondToCloseOutputFiles(FileBlock const&) override;
    bool implDoBegin(RunPrincipal&, ModuleContext const&) override;
    bool implDoEnd(RunPrincipal&, ModuleContext const&) override;
    bool implDoBegin(SubRunPrincipal&, ModuleContext const&) override;
    bool implDoEnd(SubRunPrincipal&, ModuleContext const&) override;
    bool implDoProcess(EventPrincipal&, ModuleContext const&) override;

    // A module is co-owned by one worker per schedule.  Only
    // replicated modules have a one-to-one correspondence with their
    // worker.
    std::shared_ptr<T> module_;
  };

  namespace detail {
    class SharedModule;
  }

  // This is called directly by the make_worker function created by
  // the DEFINE_ART_MODULE macro.
  template <typename T>
  WorkerT<T>::WorkerT(std::shared_ptr<T> module,
                      ModuleDescription const& md,
                      WorkerParams const& wp)
    : Worker{md, wp}, module_{module}
  {
    if (wp.scheduleID_ == ScheduleID::first()) {
      // We only want to register the products (and any shared
      // resources) once, not once for every schedule...
      module_->registerProducts(wp.producedProducts_, md);
      if constexpr (std::is_base_of_v<detail::SharedModule, T>) {
        wp.resources_.registerSharedResources(module_->sharedResources());
      }
    } else {
      // ...but we need to fill product descriptions for each module
      // copy.
      module_->fillDescriptions(md);
    }
  }

  template <typename T>
  std::string
  WorkerT<T>::workerType() const
  {
    return module_->workerType();
  }

  template <typename T>
  hep::concurrency::SerialTaskQueueChain*
  WorkerT<T>::implSerialTaskQueueChain() const
  {
    if constexpr (std::is_base_of_v<detail::SharedModule, T>) {
      return module_->serialTaskQueueChain();
    }
    return nullptr;
  }

  template <typename T>
  void
  WorkerT<T>::implBeginJob(detail::SharedResources const& resources)
  {
    module_->doBeginJob(resources);
  }

  template <typename T>
  void
  WorkerT<T>::implEndJob()
  {
    module_->doEndJob();
  }

  template <typename T>
  void
  WorkerT<T>::implRespondToOpenInputFile(FileBlock const& fb)
  {
    module_->doRespondToOpenInputFile(fb);
  }

  template <typename T>
  void
  WorkerT<T>::implRespondToCloseInputFile(FileBlock const& fb)
  {
    module_->doRespondToCloseInputFile(fb);
  }

  template <typename T>
  void
  WorkerT<T>::implRespondToOpenOutputFiles(FileBlock const& fb)
  {
    module_->doRespondToOpenOutputFiles(fb);
  }

  template <typename T>
  void
  WorkerT<T>::implRespondToCloseOutputFiles(FileBlock const& fb)
  {
    module_->doRespondToCloseOutputFiles(fb);
  }

  template <typename T>
  bool
  WorkerT<T>::implDoBegin(RunPrincipal& rp, ModuleContext const& mc)
  {
    return module_->doBeginRun(rp, mc);
  }

  template <typename T>
  bool
  WorkerT<T>::implDoEnd(RunPrincipal& rp, ModuleContext const& mc)
  {
    return module_->doEndRun(rp, mc);
  }

  template <typename T>
  bool
  WorkerT<T>::implDoBegin(SubRunPrincipal& srp, ModuleContext const& mc)
  {
    return module_->doBeginSubRun(srp, mc);
  }

  template <typename T>
  bool
  WorkerT<T>::implDoEnd(SubRunPrincipal& srp, ModuleContext const& mc)
  {
    return module_->doEndSubRun(srp, mc);
  }

  template <typename T>
  bool
  WorkerT<T>::implDoProcess(EventPrincipal& ep, ModuleContext const& mc)
  {
    // Note, only filters ever return false, and when they do it means
    // they have rejected.
    return module_->doEvent(
      ep, mc, counts_run_, counts_passed_, counts_failed_);
  }

} // namespace art

#endif /* art_Framework_Core_WorkerT_h */

// Local Variables:
// mode: c++
// End:
