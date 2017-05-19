////////////////////////////////////////////////////////////////////////
// Class:       EventIDFilter
// Plugin Type: filter (art v2_06_03)
// File:        EventIDFilter_module.cc
//
// Generated at Thu May 18 09:14:09 2017 by Kyle Knoepfel using cetskelgen
// from cetlib version v2_03_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "canvas/Utilities/EventIDMatcher.h"
#include "fhiclcpp/types/Sequence.h"

#include <string>
#include <vector>

using namespace fhicl;
using namespace std;

namespace art {
  class EventIDFilter;
}

namespace {
  std::string const parameter_comment{
    R"(The 'idsToMatch' parameter value is a sequence of patterns,
each of which are composed three fields:

  <run>:<subrun>:<event>

Each of the run, subrun, and event fields can be represented
by a number, or set of numbers.  The '*' wildcard can be used to
represent any number, and the ',' and '-' characters can be used
to sets or ranges of numbers.  For example:

   "1:*:*"     // Accept Run 1, any SubRun, any Event
   "1:2:*"     // Accept Run 1, SubRun 2, any Event
   "1:2:3"     // Accept Run 1, SubRun 2, Event 3
   "1:*:4"     // Accept Run 1, any SubRun, Event 4
   "1:2-5:*"   // Accept Run 1, SubRuns 2 through 5 (inclusive), any Event
   "*:9:10,11" // Accept any Run, SubRun 9, Events 10 and 11
   "7:2-5,8:*" // Accept Run 7, SubRuns 2 through 5 (inclusive) and 8, any Event

Specifying multiple patterns in the sequence corresponds to a
logical OR of the patterns.  In other words, if the event in question
matches any (not all) of the patterns, the event is accepted.
)" };
}

// ==============================================
class art::EventIDFilter final : public EDFilter {
public:

  struct Config {
    Sequence<string> idsToMatch{Name{"idsToMatch"}, Comment{parameter_comment}};
  };

  using Parameters = EDFilter::Table<Config>;
  explicit EventIDFilter(Parameters const& p);

  bool filter(art::Event&) override;

private:
  EventIDMatcher matcher_;
};


art::EventIDFilter::EventIDFilter(Parameters const& p) :
  matcher_{p().idsToMatch()}
{}

bool
art::EventIDFilter::filter(art::Event& e)
{
  return matcher_(e.id());
}

DEFINE_ART_MODULE(art::EventIDFilter)
