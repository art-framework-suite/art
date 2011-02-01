#ifndef Integration_ThingRawSource_h
#define Integration_ThingRawSource_h

/** \class ThingRawSource
 *
 * \version   1st Version Dec. 27, 2005

 *
 ************************************************************/

#include "art/Persistency/Provenance/EventID.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/IO/Sources/RawInputSource.h"
#include "FWCore/Integration/test/ThingAlgorithm.h"

namespace arttest {
  class ThingRawSource : public art::RawInputSource {
  public:

    // The following is not yet used, but will be the primary
    // constructor when the parameter set system is available.
    //
    explicit ThingRawSource(art::ParameterSet const& pset, art::InputSourceDescription const& desc);

    virtual ~ThingRawSource();

    virtual std::auto_ptr<art::Event> readOneEvent();

    virtual void beginRun(art::Run& r);

    virtual void endRun(art::Run& r);

    virtual void beginSubRun(art::SubRun& sr);

    virtual void endSubRun(art::SubRun& sr);

  private:
    ThingAlgorithm alg_;
    art::EventID eventID_;
  };
}
#endif
