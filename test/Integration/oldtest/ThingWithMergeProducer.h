#ifndef Integration_ThingWithMergeProducer_h
#define Integration_ThingWithMergeProducer_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDProducer.h"

#include <string>
#include <vector>

namespace arttest {
  class ThingWithMergeProducer : public art::EDProducer {
  public:

    explicit ThingWithMergeProducer(art::ParameterSet const& ps);

    virtual ~ThingWithMergeProducer();

    virtual void produce(art::Event& e, art::EventSetup const& c);

    virtual void beginRun(art::Run& r, art::EventSetup const& c);

    virtual void endRun(art::Run& r, art::EventSetup const& c);

    virtual void beginSubRun(art::SubRun& lb, art::EventSetup const& c);

    virtual void endSubRun(art::SubRun& lb, art::EventSetup const& c);

  private:

    typedef std::vector<std::string>::const_iterator Iter;

    bool changeIsEqualValue_;
    std::vector<std::string> labelsToGet_;
    bool noPut_;
  };
}

#endif
