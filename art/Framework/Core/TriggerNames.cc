
//

#include "art/Framework/Core/TriggerNames.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Persistency/Common/TriggerResults.h"

namespace art {

  TriggerNames::TriggerNames() : psetID_valid_(false) { }

  TriggerNames::TriggerNames(TriggerResults const& triggerResults) :
      psetID_valid_(false)
  {
    init(triggerResults);
  }

  bool
  TriggerNames::init(TriggerResults const& triggerResults) {

    if ( psetID_valid_ && psetID_ == triggerResults.parameterSetID()) {
      return false;
    }
    art::ServiceHandle<art::TriggerNamesService> tns;
    if (tns->getTrigPaths(triggerResults, triggerNames_)) {
      psetID_ = triggerResults.parameterSetID();
      psetID_valid_ = true;
    }
    // This should never happen
    else {
      throw art::Exception(art::errors::Unknown)
        << "TriggerNames::init cannot find the trigger names for\n"
           "a TriggerResults object.  This should be impossible,\n"
           "please send information to reproduce this problem to\n"
           "the ART developers.  (Actually if you started with a\n"
           "Streamer file and the release used to read the streamer\n"
           "is older than the release used to create it, this may\n"
           "occur, but that is not supported and you should not do that)\n";
    }

    unsigned int index = 0;
    for (Strings::const_iterator iName = triggerNames_.begin(),
         iEnd = triggerNames_.end();
         iName != iEnd;
         ++iName, ++index) {
      indexMap_[*iName] = index;
    }
    return true;
  }

  TriggerNames::Strings const&
  TriggerNames::triggerNames() const { return triggerNames_; }

  std::string const&
  TriggerNames::triggerName(unsigned int index) const {
    return triggerNames_.at(index);
  }

  unsigned int
  TriggerNames::triggerIndex(const std::string& name) const {
    IndexMap::const_iterator const pos = indexMap_.find(name);
    if (pos == indexMap_.end()) return indexMap_.size();
    return pos->second;
  }

  TriggerNames::Strings::size_type
  TriggerNames::size() const { return triggerNames_.size(); }
}
