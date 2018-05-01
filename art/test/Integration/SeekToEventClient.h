#ifndef art_test_Integration_SeekToEventClient_h
#define art_test_Integration_SeekToEventClient_h

//===================================================================
//
// SeekToEventClient service is a rough attempt at testing the
// expected behavior of the RootInput::seekToEvent function, which is
// used for (e.g.) event displays.
//
//===================================================================

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/types/Sequence.h"
#include "fhiclcpp/types/TupleAs.h"

#include <vector>

namespace art {
  class RootInput;
}

namespace arttest {
  class SeekToEventClient;
}

class arttest::SeekToEventClient {
public:
  struct Config {
    using to_EventID_t = art::EventID(art::RunNumber_t,
                                      art::SubRunNumber_t,
                                      art::EventNumber_t);
    fhicl::Sequence<fhicl::TupleAs<to_EventID_t>> nextEventsToProcess{
      fhicl::Name("nextEventsToProcess"),
      fhicl::Comment("Specify list of events to process AFTER the first event\n"
                     "in the file has been processed.\n")};
  };

  using Parameters = art::ServiceTable<Config>;
  SeekToEventClient(Parameters const&, art::ActivityRegistry&);

private:
  void postBeginJobWorkers(art::InputSource* input_source,
                           std::vector<art::Worker*> const&);
  void preProcessEvent(art::Event const&, art::ScheduleID);
  void postProcessEvent(art::Event const&, art::ScheduleID);

  std::vector<art::EventID> nextEventsToProcess_;
  cet::exempt_ptr<art::RootInput> input_{nullptr};
};

DECLARE_ART_SERVICE(arttest::SeekToEventClient, LEGACY)
#endif /* art_test_Integration_SeekToEventClient_h */

// Local Variables:
// mode: c++
// End:
