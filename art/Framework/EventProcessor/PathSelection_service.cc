#include "art/Framework/Services/System/PathSelection.h"
#include "art/Framework/EventProcessor/EventProcessor.h"

bool
art::PathSelection::setTriggerPathEnabled(std::string const& name, bool const enable) {
  return ep_.setTriggerPathEnabled(name, enable);
}

bool
art::PathSelection::setEndPathModuleEnabled(std::string const& label, bool const enable) {
  return ep_.setEndPathModuleEnabled(label, enable);
}
