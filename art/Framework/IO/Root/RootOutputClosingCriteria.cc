#include "art/Framework/IO/Root/RootOutputClosingCriteria.h"
#include "canvas/Utilities/Exception.h"

using EntryNumber_t = art::FileIndex::EntryNumber_t;
using seconds_t = std::chrono::seconds;

namespace {
  void config_assert(bool const cond, std::string const& msg)
  {
    if (!cond)
      throw art::Exception(art::errors::Configuration)
        << msg << '\n';
  }
}

art::FileProperties::FileProperties(unsigned const events,
                                    unsigned const subRuns,
                                    unsigned const runs,
                                    unsigned const inputFiles,
                                    unsigned const the_size,
                                    seconds_t const the_age)
  : counts_{events, subRuns, runs, inputFiles}
  , age_{the_age}
  , size_{the_size}
{}

std::ostream&
art::operator<<(std::ostream& os, FileProperties const& fp)
{
  os << "[nEvents: " << fp.nEvents()
     << ", nSubRuns: " << fp.nSubRuns()
     << ", nRuns: " << fp.nRuns()
     << ", nInputFiles: " << fp.nInputFiles()
     << ", size: " << fp.size()
     << ", age: " << fp.age().count()
     << "]";
  return os;
}

art::ClosingCriteria::ClosingCriteria(Config const& c)
  : ClosingCriteria{FileProperties{c.maxEvents(),
      c.maxSubRuns(),
      c.maxRuns(),
      c.maxInputFiles(),
      c.maxSize(),
      seconds_t{c.maxAge()}},
    c.granularity()}
{
  auto const& cc = closingCriteria_;
  config_assert(cc.nEvents() > 0, "maxEvents must be greater than 0.");
  config_assert(cc.nSubRuns() > 0, "maxSubRuns must be greater than 0.");
  config_assert(cc.nRuns() > 0, "maxRuns must be greater than 0.");
  config_assert(cc.nInputFiles() > 0, "maxInputFiles must be greater than 0.");
  config_assert(cc.size() > 0, "maxSize must be greater than 0 KiB.");
  config_assert(cc.age() > decltype(cc.age())::zero(), "maxAge must be greater than 0 seconds.");
}

art::ClosingCriteria::ClosingCriteria(FileProperties const& fp,
                                      std::string const& granularity)
  : closingCriteria_{fp}
  , granularity_{Boundary::value(granularity)}
{}

bool
art::ClosingCriteria::should_close(FileProperties const& fp) const
{
  return
    fp.size() >= closingCriteria_.size() ||
    fp.nEvents() >= closingCriteria_.nEvents() ||
    fp.nSubRuns() >= closingCriteria_.nSubRuns() ||
    fp.nRuns() >= closingCriteria_.nRuns() ||
    fp.nInputFiles() >= closingCriteria_.nInputFiles() ||
    fp.age() >= closingCriteria_.age();
}
