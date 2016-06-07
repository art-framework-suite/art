#ifndef art_Framework_Core_OutputFileSwitchBoundary_h
#define art_Framework_Core_OutputFileSwitchBoundary_h

#include "canvas/Utilities/Exception.h"
#include <stdexcept>
#include <type_traits>

namespace art {

  class Boundary {
  public:
    enum BT { Event, SubRun, Run, InputFile, Unset };

    Boundary(BT const b) : b_{b} {}

    BT operator()() const { return b_; }
    operator std::size_t() const { return static_cast<std::size_t>(b_); }

    static constexpr std::size_t NBoundaries() { return Unset; }

    static BT value(std::string const& spec)
    {
      if (spec == "Event") return Event;
      else if (spec == "SubRun") return SubRun;
      else if (spec == "Run") return Run;
      else if (spec == "InputFile") return InputFile;
      else if (spec == "Unset") return Unset;
      else
        throw art::Exception(art::errors::Configuration)
          << "Specified output-file switching boundary (\"" << spec << "\") not supported.\n"
          "Please choose from:\n"
          "   \"Event\"\n"
          "   \"SubRun\"\n"
          "   \"Run\"\n"
          "   \"InputFile\"\n"
          "   \"Unset\"";

    }

  private:
    BT b_;
  };

  inline std::ostream& operator<<(std::ostream& os, Boundary const& b)
  {
    std::string token {"Unset"};
    switch(b()) {
    case Boundary::Event:
      token = "Event";
      break;
    case Boundary::SubRun:
      token = "SubRun";
      break;
    case Boundary::Run:
      token = "Run";
      break;
    case Boundary::InputFile:
      token = "InputFile";
      break;
    case Boundary::Unset: ;
    }
    os << token;
    return os;
  }

}

#endif

// Local variables:
// mode: c++
// End:
