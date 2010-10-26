#include "art/Framework/Core/Factory.h"

#include "art/Utilities/DebugMacros.h"
#include "art/Utilities/EDMException.h"
#include "cetlib/container_algorithms.h"
#include <iostream>


using namespace cet;
using namespace std;


EDM_REGISTER_PLUGINFACTORY(art::MakerPluginFactory,"CMS EDM Framework Module");
namespace art {

  static void cleanup(const Factory::MakerMap::value_type& v)
  {
    delete v.second;
  }

  Factory Factory::singleInstance_;

  Factory::~Factory()
  {
    for_all(makers_, cleanup);
  }

  Factory::Factory(): makers_()

  {
  }

  Factory* Factory::get()
  {
    return &singleInstance_;
  }

  auto_ptr<Worker> Factory::makeWorker(const WorkerParams& p,
                                            sigc::signal<void, const ModuleDescription&>& pre,
                                            sigc::signal<void, const ModuleDescription&>& post) const
  {
    string modtype = p.pset_->get<string>("@module_type");
    FDEBUG(1) << "Factory: module_type = " << modtype << endl;
    MakerMap::iterator it = makers_.find(modtype);

    if(it == makers_.end())
      {
        auto_ptr<Maker> wm(MakerPluginFactory::get()->create(modtype));

	if(wm.get()==0)
	  throw art::Exception(errors::Configuration,"UnknownModule")
	    << "Module " << modtype
	    << " with version " << p.releaseVersion_
	    << " was not registered.\n"
	    << "Perhaps your module type is misspelled or is not a "
	    << "framework plugin.\n"
	    << "Try running EdmPluginDump to obtain a list of "
	    << "available Plugins.";

	FDEBUG(1) << "Factory:  created worker of type " << modtype << endl;

	pair<MakerMap::iterator,bool> ret =
	  makers_.insert(make_pair<string,Maker*>(modtype,wm.get()));

	//	if(ret.second==false)
	//	  throw runtime_error("Worker Factory map insert failed");

	it = ret.first;
	wm.release();
      }

    auto_ptr<Worker> w(it->second->makeWorker(p,pre,post));
    return w;
  }

}
