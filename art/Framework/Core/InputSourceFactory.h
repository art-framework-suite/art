#ifndef FWCore_Framework_InputSourceFactory_h
#define FWCOre_Framework_InputSourceFactory_h


#include "art/Framework/Core/InputSource.h"
#include "art/Framework/PluginManager/PluginFactory.h"

#include "fhiclcpp/ParameterSet.h"

#include <memory>
#include <string>


namespace art {

  typedef InputSource* (ISFunc)(fhicl::ParameterSet const&,
                                InputSourceDescription const&);

  typedef edmplugin::PluginFactory<ISFunc> InputSourcePluginFactory;

  class InputSourceFactory {
  public:
    ~InputSourceFactory();

    static InputSourceFactory* get();

    std::auto_ptr<InputSource>
      makeInputSource(fhicl::ParameterSet const&,
                      InputSourceDescription const&) const;


  private:
    InputSourceFactory();
    static InputSourceFactory singleInstance_;
  };

}  // namespace art

#endif  // FWCore_Framework_InputSourceFactory_h
