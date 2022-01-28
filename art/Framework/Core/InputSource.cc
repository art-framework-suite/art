#include "art/Framework/Core/InputSource.h"
// vim: set sw=2 expandtab :

namespace art {

  InputSource::~InputSource() = default;

  InputSource::ProcessingMode
  InputSource::mode(std::string const& modeString)
  {
    if (modeString == "Runs") {
      return Runs;
    }
    if (modeString == "RunsAndSubRuns") {
      return RunsAndSubRuns;
    }
    if (modeString == "RunsSubRunsAndEvents") {
      return RunsSubRunsAndEvents;
    }
    throw Exception(errors::Configuration)
      << "The 'processingMode' parameter for sources has an illegal value '"
      << modeString << "'\n"
      << "Legal values are 'Runs', 'RunsAndSubRuns', or "
         "'RunsSubRunsAndEvents'.\n";
  }

  InputSource::InputSource(ModuleDescription const& md) : moduleDescription_{md}
  {}

  ModuleDescription const&
  InputSource::moduleDescription() const
  {
    return moduleDescription_;
  }

  ProcessConfiguration const&
  InputSource::processConfiguration() const
  {
    return moduleDescription_.processConfiguration();
  }

  void
  InputSource::skipEvents(int)
  {
    throw Exception(errors::Configuration)
      << "The application has tried to peform random access on an input "
         "source\n"
      << "that does not support random access. Please reconfigure the program\n"
      << "to use an input source that supports random access (e.g. "
         "RootInput)\n";
  }

  void
  InputSource::doBeginJob()
  {}

  void
  InputSource::doEndJob()
  {}

} // namespace art
