#include "art/Framework/Core/ResultsProducer.h"

std::string const
cet::PluginTypeDeducer<art::ResultsProducer>::
value = "ResultsProducer";

void
art::ResultsProducer::
beginJob()
{
}

void
art::ResultsProducer::
endJob()
{
}

void
art::ResultsProducer::
beginSubRun(SubRun const &)
{
}

void
art::ResultsProducer::
endSubRun(SubRun const &)
{
}

void
art::ResultsProducer::
beginRun(Run const &)
{
}

void
art::ResultsProducer::
endRun(Run const &)
{
}

void
art::ResultsProducer::
event(Event const &)
{
}

void
art::ResultsProducer::
readResults(Results const &)
{
}
