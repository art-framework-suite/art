#ifndef FWCore_Framework_Factory_h
#define FWCore_Framework_Factory_h

#include "art/Framework/PluginManager/PluginFactory.h"
#include "art/Framework/Core/Worker.h"
#include "art/Framework/Core/WorkerMaker.h"
#include "art/Framework/Core/WorkerParams.h"

#include <map>
#include <string>
#include <memory>
#include "sigc++/signal.h"

namespace edm {
  typedef edmplugin::PluginFactory<Maker* ()> MakerPluginFactory;

  class Factory
  {
  public:
    typedef std::map<std::string, Maker*> MakerMap;

    ~Factory();

    static Factory* get();

    std::auto_ptr<Worker> makeWorker(const WorkerParams&,
                                     sigc::signal<void, const ModuleDescription&>& pre,
                                     sigc::signal<void, const ModuleDescription&>& post) const;


  private:
    Factory();
    static Factory singleInstance_;
    mutable MakerMap makers_;
  };

}
#endif
