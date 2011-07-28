#ifndef art_Framework_Core_WorkerT_h
#define art_Framework_Core_WorkerT_h

/*----------------------------------------------------------------------

WorkerT: Code common to all workers.

----------------------------------------------------------------------*/


#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/Worker.h"
#include "art/Framework/Core/WorkerParams.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"
#include <iosfwd>


namespace art {

  template <typename T>
  class WorkerT : public Worker {
  public:
    typedef T ModuleType;
    typedef WorkerT<T> WorkerType;
    WorkerT(std::auto_ptr<T>,
            ModuleDescription const&,
            WorkerParams const&);

    virtual ~WorkerT();

    virtual void reconfigure(fhicl::ParameterSet const &pset) {
      module_->reconfigure(pset); }

    virtual bool modifiesEvent() const { return module_->modifiesEvent(); }

  template <typename ModType>
  static std::auto_ptr<T> makeModule(ModuleDescription const& md,
                                     fhicl::ParameterSet const& pset) {
    std::auto_ptr<ModType> module = std::auto_ptr<ModType>(new ModType(pset));
    return std::auto_ptr<T>(module.release());
  }


  protected:
    T & module() {return *module_;}
    T const& module() const {return *module_;}

  private:
    virtual bool implDoBegin(EventPrincipal& ep,
                            CurrentProcessingContext const* cpc);
    virtual bool implDoEnd(EventPrincipal& ep,
                            CurrentProcessingContext const* cpc);
    virtual bool implDoBegin(RunPrincipal& rp,
                            CurrentProcessingContext const* cpc);
    virtual bool implDoEnd(RunPrincipal& rp,
                            CurrentProcessingContext const* cpc);
    virtual bool implDoBegin(SubRunPrincipal& srp,
                            CurrentProcessingContext const* cpc);
    virtual bool implDoEnd(SubRunPrincipal& srp,
                            CurrentProcessingContext const* cpc);
    virtual void implBeginJob() ;
    virtual void implEndJob() ;
    virtual void implRespondToOpenInputFile(FileBlock const& fb);
    virtual void implRespondToCloseInputFile(FileBlock const& fb);
    virtual void implRespondToOpenOutputFiles(FileBlock const& fb);
    virtual void implRespondToCloseOutputFiles(FileBlock const& fb);
    virtual std::string workerType() const;

    std::shared_ptr<T> module_;
  };

  template <typename T>
  inline
  WorkerT<T>::WorkerT(std::auto_ptr<T> ed,
                 ModuleDescription const& md,
                 WorkerParams const& wp) :
    Worker(md, wp),
    module_(ed) {
    module_->setModuleDescription(md);
    module_->registerAnyProducts(module_, wp.reg_);
  }

  template <typename T>
  WorkerT<T>::~WorkerT() {
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
