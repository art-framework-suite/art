#include "art/Framework/IO/Root/FastCloningInfoProvider.h"
#include "art/Framework/IO/Root/RootInput.h"
#include "art/Utilities/Exception.h"

art::FastCloningInfoProvider::
FastCloningInfoProvider(cet::exempt_ptr<RootInput> input) :
  input_(input)
{ }

off_t
art::FastCloningInfoProvider::remainingEvents() const
{
  if (!fastCloningPermitted()) {
    throw Exception(errors::LogicError)
        << "FastCloningInfoProvider::remainingEvents() has no meaning"
        << " in this context:\n"
        << "Check fastCloningPermitted() first.\n";
  }
  return input_->remainingEvents();
}

off_t
art::FastCloningInfoProvider::remainingSubRuns() const
{
  if (!fastCloningPermitted()) {
    throw Exception(errors::LogicError)
        << "FastCloningInfoProvider::remainingSubRuns() has no meaning"
        << " in this context:\n"
        << "Check fastCloningPermitted() first.\n";
  }
  return input_->remainingSubRuns();
}
