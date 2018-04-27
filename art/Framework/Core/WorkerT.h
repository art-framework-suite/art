#ifndef art_Framework_Core_WorkerT_h
#define art_Framework_Core_WorkerT_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ModuleType.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Framework/Principal/fwd.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

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
    WorkerT(T*, ModuleDescription const&, WorkerParams const&);

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
    void implBeginJob() override;
    void implEndJob() override;
    void implRespondToOpenInputFile(FileBlock const&) override;
    void implRespondToCloseInputFile(FileBlock const&) override;
    void implRespondToOpenOutputFiles(FileBlock const&) override;
    void implRespondToCloseOutputFiles(FileBlock const&) override;
    bool implDoBegin(RunPrincipal&, CurrentProcessingContext*) override;
    bool implDoEnd(RunPrincipal&, CurrentProcessingContext*) override;
    bool implDoBegin(SubRunPrincipal&, CurrentProcessingContext*) override;
    bool implDoEnd(SubRunPrincipal&, CurrentProcessingContext*) override;
    bool implDoProcess(EventPrincipal&,
                       ScheduleID const,
                       CurrentProcessingContext*) override;

    // Note: Modules are usually shared between workers
    // on different schedules, only replicated modules are not,
    // the PathManager owns them.
    T* module_;
  };

  // This is called directly by the make_worker function created by
  // the DEFINE_ART_MODULE macro.
  template <typename T>
  WorkerT<T>::WorkerT(T* module,
                      ModuleDescription const& md,
                      WorkerParams const& wp)
    : Worker{md, wp}, module_{module}
  {
    if (wp.scheduleID_ == ScheduleID::first()) {
      // We only want to register the products once, not once for
      // every schedule...
      module_->registerProducts(wp.producedProducts_, md);
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

  namespace detail {
    class SharedModule;
    template <typename T, typename = void>
    struct MaybeSharedModule {
      static std::nullptr_t
      chain(T*)
      {
        return nullptr;
      }
    };

    template <typename T>
    struct MaybeSharedModule<
      T,
      std::enable_if_t<std::is_base_of<SharedModule, T>::value>> {
      static hep::concurrency::SerialTaskQueueChain*
      chain(T const* const mod)
      {
        return mod->serialTaskQueueChain();
      }
    };
  }

  template <typename T>
  hep::concurrency::SerialTaskQueueChain*
  WorkerT<T>::implSerialTaskQueueChain() const
  {
    return detail::MaybeSharedModule<T>::chain(module_);
  }

  template <typename T>
  void
  WorkerT<T>::implBeginJob()
  {
    module_->doBeginJob();
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
  WorkerT<T>::implDoBegin(RunPrincipal& rp, CurrentProcessingContext* cpc)
  {
    return module_->doBeginRun(rp, cpc);
  }

  template <typename T>
  bool
  WorkerT<T>::implDoEnd(RunPrincipal& rp, CurrentProcessingContext* cpc)
  {
    return module_->doEndRun(rp, cpc);
  }

  template <typename T>
  bool
  WorkerT<T>::implDoBegin(SubRunPrincipal& srp, CurrentProcessingContext* cpc)
  {
    return module_->doBeginSubRun(srp, cpc);
  }

  template <typename T>
  bool
  WorkerT<T>::implDoEnd(SubRunPrincipal& srp, CurrentProcessingContext* cpc)
  {
    return module_->doEndSubRun(srp, cpc);
  }

  template <typename T>
  bool
  WorkerT<T>::implDoProcess(EventPrincipal& ep,
                            ScheduleID const scheduleID,
                            CurrentProcessingContext* cpc)
  {
    // Note, only filters ever return false, and when they do it means
    // they have rejected.
    return module_->doEvent(
      ep, scheduleID, cpc, counts_run_, counts_passed_, counts_failed_);
  }

} // namespace art

#endif /* art_Framework_Core_WorkerT_h */

// Local Variables:
// mode: c++
// End:
