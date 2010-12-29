#ifndef FWCore_Framework_WorkerMaker_h
#define FWCore_Framework_WorkerMaker_h

// ======================================================================
//
// WorkerMaker
//
// ======================================================================

#include "art/Framework/Core/WorkerParams.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "sigc++/signal.h"
#include <memory>
#include <string>

// ----------------------------------------------------------------------

namespace art {

  class Maker {
  public:
    virtual ~Maker();
    virtual std::auto_ptr<Worker> makeWorker(WorkerParams const&,
                                             sigc::signal<void, ModuleDescription const&>& iPre,
                                             sigc::signal<void, ModuleDescription const&>& iPost) const = 0;
  protected:
    ModuleDescription createModuleDescription(WorkerParams const &p) const;
    void throwConfigurationException(ModuleDescription const &md, sigc::signal<void, ModuleDescription const&>& post, cet::exception const& iException) const;
  };  // Maker

  template <class T>
  class WorkerMaker : public Maker {
  public:
    //typedef T worker_type;
    explicit WorkerMaker();
    virtual std::auto_ptr<Worker> makeWorker(WorkerParams const&,
                                     sigc::signal<void, ModuleDescription const&>&,
                                     sigc::signal<void, ModuleDescription const&>&) const;
  };  // WorkerMaker<>

  template <class T>
  WorkerMaker<T>::WorkerMaker() {
  }

  template <class T>
  std::auto_ptr<Worker> WorkerMaker<T>::makeWorker(WorkerParams const& p,
                                                   sigc::signal<void, ModuleDescription const&>& pre,
                                                   sigc::signal<void, ModuleDescription const&>& post) const {
    typedef T UserType;
    typedef typename UserType::ModuleType ModuleType;
    typedef typename UserType::WorkerType WorkerType;

    ModuleDescription md = createModuleDescription(p);

    std::auto_ptr<Worker> worker;
    try {
       pre(md);

       std::auto_ptr<ModuleType> module(WorkerType::template makeModule<UserType>(md, *p.pset_));
       worker=std::auto_ptr<Worker>(new WorkerType(module, md, p));
       post(md);
    } catch( cet::exception& iException){
       throwConfigurationException(md, post, iException);
    }
    return worker;
  }

}  // art

// ======================================================================

#endif
