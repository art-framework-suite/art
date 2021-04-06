#ifndef art_Framework_Core_OutputFileGranularity_h
#define art_Framework_Core_OutputFileGranularity_h

#include "canvas/Utilities/Exception.h"

#include <atomic>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace art {
  class Granularity {
  public: // Types
    enum BT { Event, SubRun, Run, InputFile, Job, Unset };

  public: // Static Api
    static constexpr std::size_t
    NBoundaries()
    {
      return Unset;
    }
    static BT
    value(std::string const& spec)
    {
      if (spec == "Event") {
        return Event;
      } else if (spec == "SubRun") {
        return SubRun;
      } else if (spec == "Run") {
        return Run;
      } else if (spec == "InputFile") {
        return InputFile;
      } else if (spec == "Job") {
        return Job;
      } else if (spec == "Unset") {
        return Unset;
      } else {
        throw art::Exception(art::errors::Configuration)
          << "Specified output-file switching boundary (\"" << spec
          << "\") not supported.\n"
             "Please choose from:\n"
             "   \"Event\"\n"
             "   \"SubRun\"\n"
             "   \"Run\"\n"
             "   \"InputFile\"\n"
             "   \"Job\"\n"
             "   \"Unset\"";
      }
    }

  public: // Special Member Functions
    ~Granularity() noexcept {}
    Granularity(BT const b) noexcept : b_{b} {}
    Granularity(Granularity const& rhs) noexcept : b_{rhs.b_.load()} {}
    Granularity(Granularity&& rhs) noexcept : b_{rhs.b_.load()} {}
    Granularity&
    operator=(Granularity const& rhs) noexcept
    {
      if (this != &rhs) {
        b_ = rhs.b_.load();
      }
      return *this;
    }
    Granularity&
    operator=(Granularity&& rhs) noexcept
    {
      b_ = rhs.b_.load();
      return *this;
    }

  public: // API
    BT
    operator()() const
    {
      return b_.load();
    }
    operator std::size_t() const { return static_cast<std::size_t>(b_.load()); }

  private: // Data Members
    std::atomic<BT> b_;
  };

  inline std::ostream&
  operator<<(std::ostream& os, Granularity const& b)
  {
    std::string token{"Unset"};
    switch (b()) {
    case Granularity::Event:
      token = "Event";
      break;
    case Granularity::SubRun:
      token = "SubRun";
      break;
    case Granularity::Run:
      token = "Run";
      break;
    case Granularity::InputFile:
      token = "InputFile";
      break;
    case Granularity::Job:
      token = "Job";
      break;
    case Granularity::Unset:;
    }
    os << token;
    return os;
  }

} // namespace art

#endif /* art_Framework_Core_OutputFileGranularity_h */

// Local variables:
// mode: c++
// End:
