#ifndef FWCore_Framework_WorkerT_h
#define FWCore_Framework_WorkerT_h

/*----------------------------------------------------------------------

WorkerT: Code common to all workers.



----------------------------------------------------------------------*/


#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/Worker.h"
#include "art/Framework/Core/WorkerParams.h"

#include "boost/shared_ptr.hpp"

#include <memory>


namespace edm {

  template <typename T>
  class WorkerT : public Worker {
  public:
    typedef T ModuleType;
    typedef WorkerT<T> WorkerType;
    WorkerT(std::auto_ptr<T>,
                   ModuleDescription const&,
                   WorkerParams const&);

    virtual ~WorkerT();


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
    virtual bool implDoBegin(SubRunPrincipal& lbp,
                            CurrentProcessingContext const* cpc);
    virtual bool implDoEnd(SubRunPrincipal& lbp,
                            CurrentProcessingContext const* cpc);
    virtual void implBeginJob() ;
    virtual void implEndJob() ;
    virtual void implRespondToOpenInputFile(FileBlock const& fb);
    virtual void implRespondToCloseInputFile(FileBlock const& fb);
    virtual void implRespondToOpenOutputFiles(FileBlock const& fb);
    virtual void implRespondToCloseOutputFiles(FileBlock const& fb);
    virtual std::string workerType() const;

    boost::shared_ptr<T> module_;
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
  WorkerT<T>::implDoBegin(SubRunPrincipal& lbp,
                           CurrentProcessingContext const* cpc) {
    return module_->doBeginSubRun(lbp, cpc);
  }

  template <typename T>
  bool
  WorkerT<T>::implDoEnd(SubRunPrincipal& lbp,
                           CurrentProcessingContext const* cpc) {
    return module_->doEndSubRun(lbp, cpc);
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

#endif
