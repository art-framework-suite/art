#include "art/Framework/IO/ClosingCriteria.h"
#include "canvas/Utilities/Exception.h"
// vim: set sw=2 expandtab :

#include <utility>

using namespace std;

using EntryNumber_t = art::FileIndex::EntryNumber_t;

namespace art {

  namespace {

    void
    config_assert(bool const cond, string const& msg)
    {
      if (!cond)
        throw art::Exception(art::errors::Configuration) << msg << '\n';
    }

  } // unnamed namespace

  FileProperties::~FileProperties() {}

  // Note: Cannot be noexcept because of age_!
  FileProperties::FileProperties()
    : counts_event_{0}
    , counts_subRun_{0}
    , counts_run_{0}
    , counts_inputFile_{0}
    , counts_job_{0}
    , treeEntryNumbers_event_{0}
    , treeEntryNumbers_subRun_{0}
    , treeEntryNumbers_run_{0}
    , treeEntryNumbers_inputFile_{0}
    , age_{chrono::seconds::zero()}
    , size_{0}
  {}

  // Note: Cannot be noexcept because of age_!
  FileProperties::FileProperties(unsigned const events,
                                 unsigned const subRuns,
                                 unsigned const runs,
                                 unsigned const inputFiles,
                                 unsigned const the_size,
                                 chrono::seconds const the_age)
    : counts_event_{events}
    , counts_subRun_{subRuns}
    , counts_run_{runs}
    , counts_inputFile_{inputFiles}
    , counts_job_{0}
    , treeEntryNumbers_event_{0}
    , treeEntryNumbers_subRun_{0}
    , treeEntryNumbers_run_{0}
    , treeEntryNumbers_inputFile_{0}
    , age_{the_age}
    , size_{the_size}
  {}

  // Note: Cannot be noexcept because of age_!
  FileProperties::FileProperties(FileProperties const& rhs)
    : counts_event_{rhs.counts_event_.load()}
    , counts_subRun_{rhs.counts_subRun_.load()}
    , counts_run_{rhs.counts_run_.load()}
    , counts_inputFile_{rhs.counts_inputFile_.load()}
    , counts_job_{rhs.counts_job_.load()}
    , treeEntryNumbers_event_{rhs.treeEntryNumbers_event_.load()}
    , treeEntryNumbers_subRun_{rhs.treeEntryNumbers_subRun_.load()}
    , treeEntryNumbers_run_{rhs.treeEntryNumbers_run_.load()}
    , treeEntryNumbers_inputFile_{rhs.treeEntryNumbers_inputFile_.load()}
    , age_{rhs.age_.load()}
    , size_{rhs.size_.load()}
  {}

  // Note: Cannot be noexcept because of age_!
  FileProperties::FileProperties(FileProperties&& rhs)
    : counts_event_{rhs.counts_event_.load()}
    , counts_subRun_{rhs.counts_subRun_.load()}
    , counts_run_{rhs.counts_run_.load()}
    , counts_inputFile_{rhs.counts_inputFile_.load()}
    , counts_job_{rhs.counts_job_.load()}
    , treeEntryNumbers_event_{rhs.treeEntryNumbers_event_.load()}
    , treeEntryNumbers_subRun_{rhs.treeEntryNumbers_subRun_.load()}
    , treeEntryNumbers_run_{rhs.treeEntryNumbers_run_.load()}
    , treeEntryNumbers_inputFile_{rhs.treeEntryNumbers_inputFile_.load()}
    , age_{rhs.age_.load()}
    , size_{rhs.size_.load()}
  {}

  // Note: Cannot be noexcept because of age_!
  FileProperties&
  FileProperties::operator=(FileProperties const& rhs)
  {
    if (this != &rhs) {
      counts_event_ = rhs.counts_event_.load();
      counts_subRun_ = rhs.counts_subRun_.load();
      counts_run_ = rhs.counts_run_.load();
      counts_inputFile_ = rhs.counts_inputFile_.load();
      counts_job_ = rhs.counts_job_.load();
      treeEntryNumbers_event_ = rhs.treeEntryNumbers_event_.load();
      treeEntryNumbers_subRun_ = rhs.treeEntryNumbers_subRun_.load();
      treeEntryNumbers_run_ = rhs.treeEntryNumbers_run_.load();
      treeEntryNumbers_inputFile_ = rhs.treeEntryNumbers_inputFile_.load();
      age_ = rhs.age_.load();
      size_ = rhs.size_.load();
    }
    return *this;
  }

  FileProperties&
  FileProperties::operator=(FileProperties&& rhs)
  {
    counts_event_ = rhs.counts_event_.load();
    counts_subRun_ = rhs.counts_subRun_.load();
    counts_run_ = rhs.counts_run_.load();
    counts_inputFile_ = rhs.counts_inputFile_.load();
    counts_job_ = rhs.counts_job_.load();
    treeEntryNumbers_event_ = rhs.treeEntryNumbers_event_.load();
    treeEntryNumbers_subRun_ = rhs.treeEntryNumbers_subRun_.load();
    treeEntryNumbers_run_ = rhs.treeEntryNumbers_run_.load();
    treeEntryNumbers_inputFile_ = rhs.treeEntryNumbers_inputFile_.load();
    age_ = rhs.age_.load();
    size_ = rhs.size_.load();
    return *this;
  }

  unsigned
  FileProperties::nEvents() const
  {
    return counts_event_.load();
  }

  unsigned
  FileProperties::nSubRuns() const
  {
    return counts_subRun_.load();
  }

  unsigned
  FileProperties::nRuns() const
  {
    return counts_run_.load();
  }

  unsigned
  FileProperties::nInputFiles() const
  {
    return counts_inputFile_.load();
  }

  unsigned
  FileProperties::size() const
  {
    return size_.load();
  }

  chrono::seconds
  FileProperties::age() const
  {
    return age_.load();
  }

  FileIndex::EntryNumber_t
  FileProperties::eventEntryNumber() const
  {
    return treeEntryNumbers_event_.load();
  }

  FileIndex::EntryNumber_t
  FileProperties::subRunEntryNumber() const
  {
    return treeEntryNumbers_subRun_.load();
  }

  FileIndex::EntryNumber_t
  FileProperties::runEntryNumber() const
  {
    return treeEntryNumbers_run_.load();
  }

  void
  FileProperties::updateSize(unsigned const size)
  {
    size_ = size;
  }

  void
  FileProperties::updateAge(chrono::seconds const age)
  {
    age_ = age;
  }

  void
  FileProperties::update_event()
  {
    ++treeEntryNumbers_event_;
    ++counts_event_;
  }

  void
  FileProperties::update_subRun(OutputFileStatus const status)
  {
    ++treeEntryNumbers_subRun_;
    if (status != OutputFileStatus::Switching) {
      ++counts_subRun_;
    }
  }

  void
  FileProperties::update_run(OutputFileStatus const status)
  {
    ++treeEntryNumbers_run_;
    if (status != OutputFileStatus::Switching) {
      ++counts_run_;
    }
  }

  void
  FileProperties::update_inputFile()
  {
    ++counts_inputFile_;
  }

  ostream&
  operator<<(ostream& os, FileProperties const& fp)
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
                                   string const& granularity)
    : closingCriteria_{fp}, granularity_{Granularity::value(granularity)}
  {}

  ClosingCriteria::ClosingCriteria(Config const& c)
    : ClosingCriteria{FileProperties{c.maxEvents(),
                                     c.maxSubRuns(),
                                     c.maxRuns(),
                                     c.maxInputFiles(),
                                     c.maxSize(),
                                     chrono::seconds{c.maxAge()}},
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
