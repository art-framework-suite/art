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

#include <iosfwd>
#include <memory>

namespace art {

template <typename T>
class WorkerT : public Worker {

  // Let PathManager use module() to delete the trigger results inserter.
  friend class PathManager;

public: // TYPES

  using ModuleType = T;
  //using WorkerType = WorkerT<T>;

public: // MEMBER FUNCTIONS -- Special Member Functions

  // This is called directly by the make_worker function created
  // by the DEFINE_ART_MODULE macro.
  // Note: OutputWorker overrides this.
  WorkerT(T*, ModuleDescription const&, WorkerParams const&);

//public: // MEMBER FUNCTIONS -- Public API

  //bool
  //modifiesEvent() const override
  //{
  //  return module_->modifiesEvent();
  //}

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

private: // MEMBER FUNCTIONS -- Worker API

  std::string
  workerType() const override;

  hep::concurrency::SerialTaskQueueChain*
  implSerialTaskQueueChain() const override;

  void
  implBeginJob() override;

  void
  implEndJob() override;

  void
  implRespondToOpenInputFile(FileBlock const&) override;

  void
  implRespondToCloseInputFile(FileBlock const&) override;

  void
  implRespondToOpenOutputFiles(FileBlock const&) override;

  void
  implRespondToCloseOutputFiles(FileBlock const&) override;

  bool
  implDoBegin(RunPrincipal&, CurrentProcessingContext*) override;

  bool
  implDoEnd(RunPrincipal&, CurrentProcessingContext*) override;

  bool
  implDoBegin(SubRunPrincipal&, CurrentProcessingContext*) override;

  bool
  implDoEnd(SubRunPrincipal&, CurrentProcessingContext*) override;

  bool
  implDoProcess(EventPrincipal&, int streamIndex, CurrentProcessingContext*) override;

private: // MEMBER DATA

  // Note: Modules are usually shared between workers
  // on different streams, only stream modules are not,
  // the PathManager owns them.
  T*
  module_;

};

// This is called directly by the make_worker function created
// by the DEFINE_ART_MODULE macro.
// Note: OutputWorker overrides this.
template <typename T>
WorkerT<T>::
WorkerT(T* module, ModuleDescription const& md, WorkerParams const& wp)
  : Worker{md, wp}
  , module_{module}
{
  //module_->setModuleDescription(md);
  if (wp.streamIndex_ == 0) {
    // We only want to register the products once,
    // not once for every stream!
    module_->registerProducts(wp.reg_, md);
  }
}

template <typename T>
std::string
WorkerT<T>::
workerType() const
{
  return module_->workerType();
}

template <typename T>
hep::concurrency::SerialTaskQueueChain*
WorkerT<T>::
implSerialTaskQueueChain() const
{
  return module_->serialTaskQueueChain();
}

template <typename T>
void
WorkerT<T>::
implBeginJob()
{
  module_->doBeginJob();
}

template <typename T>
void
WorkerT<T>::
implEndJob()
{
  module_->doEndJob();
}

template <typename T>
void
WorkerT<T>::
implRespondToOpenInputFile(FileBlock const& fb)
{
  module_->doRespondToOpenInputFile(fb);
}

template <typename T>
void
WorkerT<T>::
implRespondToCloseInputFile(FileBlock const& fb)
{
  module_->doRespondToCloseInputFile(fb);
}

template <typename T>
void
WorkerT<T>::
implRespondToOpenOutputFiles(FileBlock const& fb)
{
  module_->doRespondToOpenOutputFiles(fb);
}

template <typename T>
void
WorkerT<T>::
implRespondToCloseOutputFiles(FileBlock const& fb)
{
  module_->doRespondToCloseOutputFiles(fb);
}

template <typename T>
bool
WorkerT<T>::
implDoBegin(RunPrincipal& rp, CurrentProcessingContext* cpc)
{
  //cpc->setModule(module_);
  return module_->doBeginRun(rp, cpc);
}

template <typename T>
bool
WorkerT<T>::
implDoEnd(RunPrincipal& rp, CurrentProcessingContext* cpc)
{
  //cpc->setModule(module_);
  return module_->doEndRun(rp, cpc);
}

template <typename T>
bool
WorkerT<T>::
implDoBegin(SubRunPrincipal& srp, CurrentProcessingContext* cpc)
{
  //cpc->setModule(module_);
  return module_->doBeginSubRun(srp, cpc);
}

template <typename T>
bool
WorkerT<T>::
implDoEnd(SubRunPrincipal& srp, CurrentProcessingContext* cpc)
{
  //cpc->setModule(module_);
  return module_->doEndSubRun(srp, cpc);
}

template <typename T>
bool
WorkerT<T>::
implDoProcess(EventPrincipal& ep, int si, CurrentProcessingContext* cpc)
{
  //cpc->setModule(module_);
  // Note, only filters ever return false, and when they do it means they have rejected.
  return module_->doEvent(ep, si, cpc, counts_run_, counts_passed_, counts_failed_);
}

} // namespace art


#endif /* art_Framework_Core_WorkerT_h */

// Local Variables:
// mode: c++
// End:
