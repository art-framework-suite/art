#include "art/Utilities/ExceptionMessages.h"
#include "cetlib/trim.h"
#include "cetlib_except/exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <sstream>
#include <string>

using mf::LogSystem;

namespace art {
  void
  printArtException(cet::exception const& e, char const* prog) try {
    std::string programName(prog ? prog : "program");
    std::string shortDesc("ArtException");
    std::ostringstream longDesc;
    longDesc << "cet::exception caught in " << programName << "\n"
             << cet::trim_right_copy(e.explain_self(), " \n");
    LogSystem(shortDesc) << longDesc.str();
  }
  catch (...) {
  }

  void
  printBadAllocException(char const* prog) try {
    std::string programName(prog ? prog : "program");
    std::string shortDesc("std::bad_allocException");
    std::ostringstream longDesc;
    longDesc << "std::bad_alloc exception caught in " << programName << "\n"
             << "The job has probably exhausted the virtual memory available "
                "to the process.";
    LogSystem(shortDesc) << longDesc.str();
  }
  catch (...) {
  }

  void
  printStdException(std::exception const& e, char const* prog) try {
    std::string programName(prog ? prog : "program");
    std::string shortDesc("StdLibException");
    std::ostringstream longDesc;
    longDesc << "Standard library exception caught in " << programName << "\n"
             << cet::trim_right_copy(e.what(), " \n");
    LogSystem(shortDesc) << longDesc.str();
  }
  catch (...) {
  }

  void
  printUnknownException(char const* prog) try {
    std::string programName(prog ? prog : "program");
    std::string shortDesc("UnknownException");
    std::ostringstream longDesc;
    longDesc << "Unknown exception caught in " << programName;
    LogSystem(shortDesc) << longDesc.str();
  }
  catch (...) {
  }

} // art
