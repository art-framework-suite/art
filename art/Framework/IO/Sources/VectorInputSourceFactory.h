#ifndef FWCore_Sources_VectorInputSourceFactory_h
#define FWCore_Sources_VectorInputSourceFactory_h


#include "art/Framework/IO/Sources/VectorInputSource.h"
#include "art/Framework/PluginManager/PluginFactory.h"

#include "fhiclcpp/ParameterSet.h"

#include <memory>
#include <string>


namespace edm {

  class InputSourceDescription;

  typedef VectorInputSource* (ISVecFunc)(fhicl::ParameterSet const&, InputSourceDescription const&);
  typedef edmplugin::PluginFactory<ISVecFunc> VectorInputSourcePluginFactory;

  class VectorInputSourceFactory
  {
  public:
    ~VectorInputSourceFactory();

    static VectorInputSourceFactory* get();

    std::auto_ptr<VectorInputSource>
      makeVectorInputSource(fhicl::ParameterSet const&,
                       InputSourceDescription const&) const;


  private:
    VectorInputSourceFactory();
    static VectorInputSourceFactory singleInstance_;
  };

}  // namespace edm

#endif  // FWCore_Sources_VectorInputSourceFactory_h
