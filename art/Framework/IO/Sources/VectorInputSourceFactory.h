#ifndef FWCore_Sources_VectorInputSourceFactory_h
#define FWCore_Sources_VectorInputSourceFactory_h

#include "art/Framework/PluginManager/PluginFactory.h"
#include "art/Framework/IO/Sources/VectorInputSource.h"

#include <string>
#include <memory>

namespace edm {
  class ParameterSet;
  class InputSourceDescription;

  typedef VectorInputSource* (ISVecFunc)(ParameterSet const&, InputSourceDescription const&);
  typedef edmplugin::PluginFactory<ISVecFunc> VectorInputSourcePluginFactory;

  class VectorInputSourceFactory
  {
  public:
    ~VectorInputSourceFactory();

    static VectorInputSourceFactory* get();

    std::auto_ptr<VectorInputSource>
      makeVectorInputSource(ParameterSet const&,
		       InputSourceDescription const&) const;


  private:
    VectorInputSourceFactory();
    static VectorInputSourceFactory singleInstance_;
  };

}
#endif
