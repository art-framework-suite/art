#include "art/Framework/Core/EmptyEventTimestampPlugin.h"

std::string const
cet::PluginTypeDeducer<art::EmptyEventTimestampPlugin>::
value = "EmptyEventTimestampPlugin";

art::EmptyEventTimestampPlugin::
EmptyEventTimestampPlugin(fhicl::ParameterSet const &)
:
  lastEventTimestamp_(Timestamp::invalidTimestamp())
{
}
