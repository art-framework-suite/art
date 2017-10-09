#include "art/Framework/IO/Root/RootOutputClosingCriteria.h"
// vim: set sw=2 expandtab :

#include "canvas/Utilities/Exception.h"

using EntryNumber_t = art::FileIndex::EntryNumber_t;
using seconds_t = std::chrono::seconds;

namespace art {

  namespace {
    void
    config_assert(bool const cond, std::string const& msg)
    {
      if (!cond)
        throw art::Exception(art::errors::Configuration) << msg << '\n';
    }

  } // unnamed namespace

  FileProperties::~FileProperties() {}

  FileProperties::FileProperties()
    : counts_{{}}
    , treeEntryNumbers_{{}}
    , age_{std::chrono::seconds::zero()}
    , size_{}
  {}

  FileProperties::FileProperties(unsigned const events,
                                 unsigned const subRuns,
                                 unsigned const runs,
                                 unsigned const inputFiles,
                                 unsigned const the_size,
                                 seconds_t const the_age)
    : counts_{events, subRuns, runs, inputFiles}
    , treeEntryNumbers_{{}}
    , age_{the_age}
    , size_{the_size}
  {}

  unsigned
  FileProperties::nEvents() const
  {
    return counts_[Granularity::Event];
  }

  unsigned
  FileProperties::nSubRuns() const
  {
    return counts_[Granularity::SubRun];
  }

  unsigned
  FileProperties::nRuns() const
  {
    return counts_[Granularity::Run];
  }

  unsigned
  FileProperties::nInputFiles() const
  {
    return counts_[Granularity::InputFile];
  }

  unsigned
  FileProperties::size() const
  {
    return size_;
  }

  std::chrono::seconds
  FileProperties::age() const
  {
    return age_;
  }

  FileIndex::EntryNumber_t
  FileProperties::eventEntryNumber() const
  {
    return treeEntryNumbers_[Granularity::Event];
  }

  FileIndex::EntryNumber_t
  FileProperties::subRunEntryNumber() const
  {
    return treeEntryNumbers_[Granularity::SubRun];
  }

  FileIndex::EntryNumber_t
  FileProperties::runEntryNumber() const
  {
    return treeEntryNumbers_[Granularity::Run];
  }

  void
  FileProperties::updateSize(unsigned const size)
  {
    size_ = size;
  }

  void
  FileProperties::updateAge(std::chrono::seconds const age)
  {
    age_ = age;
  }

  void
  FileProperties::update_event()
  {
    ++treeEntryNumbers_[Granularity::Event];
    ++counts_[Granularity::Event];
  }

  void
  FileProperties::update_subRun(OutputFileStatus const status)
  {
    ++treeEntryNumbers_[Granularity::SubRun];
    if (status != OutputFileStatus::Switching) {
      ++counts_[Granularity::SubRun];
    }
  }

  void
  FileProperties::update_run(OutputFileStatus const status)
  {
    ++treeEntryNumbers_[Granularity::Run];
    if (status != OutputFileStatus::Switching) {
      ++counts_[Granularity::Run];
    }
  }

  void
  FileProperties::update_inputFile()
  {
    ++counts_[Granularity::InputFile];
  }

  std::ostream&
  operator<<(std::ostream& os, FileProperties const& fp)
  {
    os << "[nEvents: " << fp.nEvents() << ", nSubRuns: " << fp.nSubRuns()
       << ", nRuns: " << fp.nRuns() << ", nInputFiles: " << fp.nInputFiles()
       << ", size: " << fp.size() << ", age: " << fp.age().count() << "]";
    return os;
  }

  ClosingCriteria::~ClosingCriteria() {}

  ClosingCriteria::ClosingCriteria()
    : closingCriteria_{}
    , granularity_{
        Granularity::value(ClosingCriteria::Defaults::granularity_default())}
  {}

  ClosingCriteria::ClosingCriteria(FileProperties const& fp,
                                   std::string const& granularity)
    : closingCriteria_{fp}, granularity_{Granularity::value(granularity)}
  {}

  ClosingCriteria::ClosingCriteria(Config const& c)
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
    config_assert(cc.nInputFiles() > 0,
                  "maxInputFiles must be greater than 0.");
    config_assert(cc.size() > 0, "maxSize must be greater than 0 KiB.");
    config_assert(cc.age() > decltype(cc.age())::zero(),
                  "maxAge must be greater than 0 seconds.");
  }

  FileProperties const&
  ClosingCriteria::fileProperties() const
  {
    return closingCriteria_;
  }

  Granularity
  ClosingCriteria::granularity() const
  {
    return granularity_;
  }

  bool
  ClosingCriteria::should_close(FileProperties const& fp) const
  {
    return (fp.size() >= closingCriteria_.size()) ||
           (fp.nEvents() >= closingCriteria_.nEvents()) ||
           (fp.nSubRuns() >= closingCriteria_.nSubRuns()) ||
           (fp.nRuns() >= closingCriteria_.nRuns()) ||
           (fp.nInputFiles() >= closingCriteria_.nInputFiles()) ||
           (fp.age() >= closingCriteria_.age());
  }

} // namespace art
