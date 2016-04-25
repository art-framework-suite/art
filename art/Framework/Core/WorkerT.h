#ifndef art_Framework_Core_WorkerT_h
#define art_Framework_Core_WorkerT_h

/*----------------------------------------------------------------------

WorkerT: Code common to all workers.

----------------------------------------------------------------------*/


#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "fhiclcpp/ParameterSet.h"

#include <iosfwd>
#include <memory>

namespace art {

  template <typename T>
  class WorkerT : public Worker {
  public:
    typedef T ModuleType;
    typedef WorkerT<T> WorkerType;
    WorkerT(std::unique_ptr<T> &&,
            ModuleDescription const&,
            WorkerParams const&);

    void reconfigure(fhicl::ParameterSet const &pset) override {
      module_->reconfigure(pset); }

    bool modifiesEvent() const override { return module_->modifiesEvent(); }

    template <typename ModType>
    static std::unique_ptr<T> makeModule(ModuleDescription const& md,
                                         fhicl::ParameterSet const& pset) {
      std::unique_ptr<ModType> module = std::unique_ptr<ModType>(new ModType(pset));
      return std::unique_ptr<T>(module.release());
    }

  protected:
    T & module() {return *module_;}
    T const& module() const {return *module_;}

  private:

    bool implDoBegin(EventPrincipal& ep,
                     CurrentProcessingContext const* cpc) override;
    bool implDoEnd(EventPrincipal& ep,
                   CurrentProcessingContext const* cpc) override;
    bool implDoBegin(RunPrincipal& rp,
                     CurrentProcessingContext const* cpc) override;
    bool implDoEnd(RunPrincipal& rp,
                   CurrentProcessingContext const* cpc) override;
    bool implDoBegin(SubRunPrincipal& srp,
                     CurrentProcessingContext const* cpc) override;
    bool implDoEnd(SubRunPrincipal& srp,
                   CurrentProcessingContext const* cpc) override;
    void implBeginJob()  override;
    void implEndJob()  override;
    void implRespondToOpenInputFile(FileBlock const& fb) override;
    void implRespondToCloseInputFile(FileBlock const& fb) override;
    void implRespondToOpenOutputFiles(FileBlock const& fb) override;
    void implRespondToCloseOutputFiles(FileBlock const& fb) override;
    std::string workerType() const override;

    std::shared_ptr<T> module_;
  };

  template <typename T>
  inline
  WorkerT<T>::WorkerT(std::unique_ptr<T> && ed,
                      ModuleDescription const& md,
                      WorkerParams const& wp) :
    Worker{md, wp},
    module_{ed.release()}
  {
    module_->setModuleDescription(md);
    module_->registerProducts(wp.reg_, md);
  }

  template <typename T>
  bool
  WorkerT<T>::implDoBegin(EventPrincipal& ep,
                          CurrentProcessingContext const* cpc) {
    return module_->doEvent(ep, cpc);
  }

  template <typename T>
  bool
  WorkerT<T>::implDoEnd(EventPrincipal& ,
                        CurrentProcessingContext const*) {
    return false;
  }

  template <typename T>
  bool
  WorkerT<T>::implDoBegin(RunPrincipal& rp,
                          CurrentProcessingContext const* cpc) {
    return module_->doBeginRun(rp, cpc);
  }

  template <typename T>
  bool
  WorkerT<T>::implDoEnd(RunPrincipal& rp,
                        CurrentProcessingContext const* cpc) {
    return module_->doEndRun(rp, cpc);
  }

  template <typename T>
  bool
  WorkerT<T>::implDoBegin(SubRunPrincipal& srp,
                          CurrentProcessingContext const* cpc) {
    return module_->doBeginSubRun(srp, cpc);
  }

  template <typename T>
  bool
  WorkerT<T>::implDoEnd(SubRunPrincipal& srp,
                        CurrentProcessingContext const* cpc) {
    return module_->doEndSubRun(srp, cpc);
  }

  template <typename T>
  std::string
  WorkerT<T>::workerType() const {
    return module_->workerType();
  }

  template <typename T>
  void
  WorkerT<T>::implBeginJob() {
    module_->doBeginJob();
  }

  template <typename T>
  void
  WorkerT<T>::implEndJob() {
    module_->doEndJob();
  }

  template <typename T>
  void
  WorkerT<T>::implRespondToOpenInputFile(FileBlock const& fb) {
    module_->doRespondToOpenInputFile(fb);
  }

  template <typename T>
  void
  WorkerT<T>::implRespondToCloseInputFile(FileBlock const& fb) {
    module_->doRespondToCloseInputFile(fb);
  }

  template <typename T>
  void
  WorkerT<T>::implRespondToOpenOutputFiles(FileBlock const& fb) {
    module_->doRespondToOpenOutputFiles(fb);
  }

  template <typename T>
  void
  WorkerT<T>::implRespondToCloseOutputFiles(FileBlock const& fb) {
    module_->doRespondToCloseOutputFiles(fb);
  }
}

#endif /* art_Framework_Core_WorkerT_h */

// Local Variables:
// mode: c++
// End:
