#ifndef art_Framework_Core_WorkerT_h
#define art_Framework_Core_WorkerT_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/ModuleBase.h"
#include "art/Framework/Core/fwd.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/SharedResource.h"
#include "cetlib/exempt_ptr.h"

#include <memory>
#include <type_traits>

namespace art {
  class Modifier;
  template <typename T>
  class WorkerT : public Worker {
  public:
    WorkerT(T*, WorkerParams const&);

  private:
    hep::concurrency::SerialTaskQueueChain* doSerialTaskQueueChain()
      const override;
    void doBeginJob(detail::SharedResources const&) override;
    void doEndJob() override;
    void doRespondToOpenInputFile(FileBlock const&) override;
    void doRespondToCloseInputFile(FileBlock const&) override;
    void doRespondToOpenOutputFiles(FileBlock const&) override;
    void doRespondToCloseOutputFiles(FileBlock const&) override;
    void doBegin(RunPrincipal&, ModuleContext const&) override;
    void doEnd(RunPrincipal&, ModuleContext const&) override;
    void doBegin(SubRunPrincipal&, ModuleContext const&) override;
    void doEnd(SubRunPrincipal&, ModuleContext const&) override;
    bool doProcess(EventPrincipal&, ModuleContext const&) override;

    // A module is co-owned by one worker per schedule.  Only
    // replicated modules have a one-to-one correspondence with their
    // worker.
    cet::exempt_ptr<T> module_;
  };

  namespace detail {
    class SharedModule;
  }

  // This is called directly by the make_worker function created by
  // the DEFINE_ART_MODULE macro.
  template <typename T>
  WorkerT<T>::WorkerT(T* module, WorkerParams const& wp)
    : Worker{module->moduleDescription(), wp}, module_{module}
  {
    if constexpr (std::is_base_of_v<Modifier, T>) {
      if (wp.scheduleID_ == ScheduleID::first()) {
        // We only want to register the products, not once for every
        // schedule...
        module_->registerProducts(wp.producedProducts_);
      } else {
        // ...but we need to fill product descriptions for each module
        // copy.
        module_->fillProductDescriptions();
      }
    }

    // Register shared resources only once
    if constexpr (std::is_base_of_v<detail::SharedModule, T>) {
      if (wp.scheduleID_ == ScheduleID::first()) {
        wp.resources_.registerSharedResources(module_->sharedResources());
      }
    }
  }

  template <typename T>
  hep::concurrency::SerialTaskQueueChain*
  WorkerT<T>::doSerialTaskQueueChain() const
  {
    if constexpr (std::is_base_of_v<detail::SharedModule, T>) {
      return module_->serialTaskQueueChain();
    } else {
      return nullptr;
    }
  }

  template <typename T>
  void
  WorkerT<T>::doBeginJob(detail::SharedResources const& resources)
  {
    module_->doBeginJob(resources);
  }

  template <typename T>
  void
  WorkerT<T>::doEndJob()
  {
    module_->doEndJob();
  }

  template <typename T>
  void
  WorkerT<T>::doRespondToOpenInputFile(FileBlock const& fb)
  {
    module_->doRespondToOpenInputFile(fb);
  }

  template <typename T>
  void
  WorkerT<T>::doRespondToCloseInputFile(FileBlock const& fb)
  {
    module_->doRespondToCloseInputFile(fb);
  }

  template <typename T>
  void
  WorkerT<T>::doRespondToOpenOutputFiles(FileBlock const& fb)
  {
    module_->doRespondToOpenOutputFiles(fb);
  }

  template <typename T>
  void
  WorkerT<T>::doRespondToCloseOutputFiles(FileBlock const& fb)
  {
    module_->doRespondToCloseOutputFiles(fb);
  }

  template <typename T>
  void
  WorkerT<T>::doBegin(RunPrincipal& rp, ModuleContext const& mc)
  {
    module_->doBeginRun(rp, mc);
  }

  template <typename T>
  void
  WorkerT<T>::doEnd(RunPrincipal& rp, ModuleContext const& mc)
  {
    module_->doEndRun(rp, mc);
  }

  template <typename T>
  void
  WorkerT<T>::doBegin(SubRunPrincipal& srp, ModuleContext const& mc)
  {
    module_->doBeginSubRun(srp, mc);
  }

  template <typename T>
  void
  WorkerT<T>::doEnd(SubRunPrincipal& srp, ModuleContext const& mc)
  {
    module_->doEndSubRun(srp, mc);
  }

  template <typename T>
  bool
  WorkerT<T>::doProcess(EventPrincipal& ep, ModuleContext const& mc)
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
