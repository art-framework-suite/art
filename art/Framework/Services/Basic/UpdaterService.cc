#include "art/Framework/Services/Basic/UpdaterService.h"

#include "art/Persistency/Common/Trie.h"
#include "art/Persistency/Provenance/EventID.h"

#include "MessageFacility/MessageLogger.h"

#include <iostream>


UpdaterService::UpdaterService(const fhicl::ParameterSet & cfg, edm::ActivityRegistry & r ) :
  theEventId(0) {
  r.watchPreProcessEvent( this, & UpdaterService::init );
  theInit();
}

UpdaterService::~UpdaterService() {
}

void UpdaterService::init(const edm::EventID& eId, const edm::Timestamp&) {
  theInit();
  theEventId = &eId;
}

void UpdaterService::theInit() {
  theCounts.clear();
}

bool UpdaterService::checkOnce(std::string tag) {
  bool answer=true;

  std::map<std::string, uint>::iterator i=theCounts.find(tag);
  if (i!=theCounts.end()) {
    i->second++;
    answer=false;
  }
  else{
    theCounts[tag]=1;
    answer=true;
  }

  if (theEventId) {
    mf::LogDebug("UpdaterService")
      << "checking ONCE on tag: " << tag
      << "on run: " << theEventId->run()
      << " event: " << theEventId->event()
      << ((answer)?" -> true":" -> false");
  }
  return answer;
}

bool UpdaterService::check(std::string tag, std::string label) {
  return true;
}
