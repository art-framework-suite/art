#include "art/Framework/Core/InputSource.h"
// vim: set sw=2 expandtab :

namespace art {

InputSource::
~InputSource() noexcept
{
}

InputSource::
InputSource(ModuleDescription const& md)
  : moduleDescription_{md}
{
}

ModuleDescription const&
InputSource::
moduleDescription() const
{
  return moduleDescription_;
}

ProcessConfiguration const&
InputSource::
processConfiguration() const
{
  return moduleDescription_.processConfiguration();
}

std::unique_ptr<EventPrincipal>
InputSource::
readEvent(EventID const&)
{
  throw Exception(errors::Configuration)
      << "The application has tried to peform random access on an input source\n"
      << "that does not support random access. Please reconfigure the program\n"
      << "to use an input source that supports random access (e.g. RootInput)\n";
}

void
InputSource::
skipEvents(int)
{
  throw Exception(errors::Configuration)
      << "The application has tried to peform random access on an input source\n"
      << "that does not support random access. Please reconfigure the program\n"
      << "to use an input source that supports random access (e.g. RootInput)\n";
}

void
InputSource::
rewind()
{
  throw Exception(errors::Configuration)
      << "The application has tried to rewind an input source\n"
      << "that does not support rewinding. Please reconfigure the program\n"
      << "to use an input source that supports random access (e.g. RootInput)\n";
}

void
InputSource::
doBeginJob()
{
}

void
InputSource::
doEndJob()
{
}

} // namespace art

