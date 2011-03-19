#include "art/Framework/Core/InputSource.h"

art::InputSource::~InputSource() { }

std::auto_ptr<art::EventPrincipal>
art::InputSource::readEvent(art::EventID const&)
{
  throw art::Exception(art::errors::Configuration)
    << "The application has tried to peform random access on an input source\n"
    << "that does not support random access. Please reconfigure the program\n"
    << "to use an input source that supports random access (e.g. RootInput)\n";
}

void
art::InputSource::skipEvents(int)
{
  throw art::Exception(art::errors::Configuration)
    << "The application has tried to peform random access on an input source\n"
    << "that does not support random access. Please reconfigure the program\n"
    << "to use an input source that supports random access (e.g. RootInput)\n";
}

void
art::InputSource::rewind()
{
  throw art::Exception(art::errors::Configuration)
    << "The application has tried to rewind an input source\n"
    << "that does not support rewinding. Please reconfigure the program\n"
    << "to use an input source that supports random access (e.g. RootInput)\n";
}

void
art::InputSource::doBeginJob()
{ }

void
art::InputSource::doEndJob()
{ }

