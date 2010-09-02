//
// Package:     Modules
// Class  :     IterateNTimesLooper
//

// user include files
#include "art/Framework/Modules/IterateNTimesLooper.h"
#include "art/ParameterSet/ParameterSet.h"


//
// constructors and destructor
//
IterateNTimesLooper::IterateNTimesLooper(const edm::ParameterSet& iConfig) :
max_(iConfig.getParameter<unsigned int>("nTimes")),
times_(0),
shouldStop_(false)
{ }

IterateNTimesLooper::~IterateNTimesLooper()
{ }

//
// member functions
//
void
IterateNTimesLooper::startingNewLoop(unsigned int iIteration) {
  times_ = iIteration;
  if (iIteration >= max_ ) {
    shouldStop_ = true;
  }
}

edm::EDLooper::Status
IterateNTimesLooper::duringLoop(const edm::Event& event) {
  return shouldStop_ ? kStop : kContinue;
}

edm::EDLooper::Status
IterateNTimesLooper::endOfLoop(unsigned int iCounter) {
  ++times_;
  return (times_ < max_ ) ? kContinue : kStop;
}
