#ifndef Integration_ThingExtSource_h
#define Integration_ThingExtSource_h

/** \class ThingExtSource
 *
 * \version   1st Version Dec. 27, 2005

 *
 ************************************************************/

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/IO/Sources/ExternalInputSource.h"
#include "FWCore/Integration/test/ThingAlgorithm.h"

namespace arttest {
  class ThingExtSource : public art::ExternalInputSource {
  public:

    // The following is not yet used, but will be the primary
    // constructor when the parameter set system is available.
    //
    explicit ThingExtSource(art::ParameterSet const& pset, art::InputSourceDescription const& desc);

    virtual ~ThingExtSource();

    virtual bool produce(art::Event& e);

    virtual void beginRun(art::Run& r);

    virtual void endRun(art::Run& r);

    virtual void beginSubRun(art::SubRun& lb);

    virtual void endSubRun(art::SubRun& lb);

  private:
    ThingAlgorithm alg_;
  };
}
#endif
