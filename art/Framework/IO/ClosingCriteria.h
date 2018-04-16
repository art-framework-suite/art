#ifndef art_Framework_IO_ClosingCriteria_h
#define art_Framework_IO_ClosingCriteria_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/OutputFileGranularity.h"
#include "art/Framework/Core/OutputFileStatus.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/FileIndex.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalAtom.h"

#include <atomic>
#include <chrono>
#include <type_traits>

namespace art {
  class FileProperties {
  public:
    ~FileProperties();
    // Note: Cannot be noexcept because of age_!
    FileProperties();
    // Note: Cannot be noexcept because of age_!
    FileProperties(unsigned events,
                   unsigned subRuns,
                   unsigned runs,
                   unsigned inputFiles,
                   unsigned size,
                   std::chrono::seconds age);
    // Note: Cannot be noexcept because of age_!
    FileProperties(FileProperties const&);
    // Note: Cannot be noexcept because of age_!
    FileProperties(FileProperties&&);
    // Note: Cannot be noexcept because of age_!
    FileProperties& operator=(FileProperties const&);
    // Note: Cannot be noexcept because of age_!
    FileProperties& operator=(FileProperties&&);
    unsigned nEvents() const;
    unsigned nSubRuns() const;
    unsigned nRuns() const;
    unsigned nInputFiles() const;
    unsigned size() const;
    std::chrono::seconds age() const;
    FileIndex::EntryNumber_t eventEntryNumber() const;
    FileIndex::EntryNumber_t subRunEntryNumber() const;
    FileIndex::EntryNumber_t runEntryNumber() const;
    void update_event();
    void update_subRun(OutputFileStatus const status);
    void update_run(OutputFileStatus const status);
    void update_inputFile();
    void updateSize(unsigned const size);
    void updateAge(std::chrono::seconds const age);

  private:
    std::atomic<unsigned> counts_event_;
    std::atomic<unsigned> counts_subRun_;
    std::atomic<unsigned> counts_run_;
    std::atomic<unsigned> counts_inputFile_;
    std::atomic<unsigned> counts_job_;
    std::atomic<FileIndex::EntryNumber_t> treeEntryNumbers_event_;
    std::atomic<FileIndex::EntryNumber_t> treeEntryNumbers_subRun_;
    std::atomic<FileIndex::EntryNumber_t> treeEntryNumbers_run_;
    std::atomic<FileIndex::EntryNumber_t> treeEntryNumbers_inputFile_;
    std::atomic<std::chrono::seconds> age_;
    std::atomic<unsigned> size_;
  };

  std::ostream& operator<<(std::ostream& os, FileProperties const& fp);

  class ClosingCriteria {
  public:
    struct Defaults {
      static constexpr unsigned
      unsigned_max()
      {
        return std::numeric_limits<unsigned>::max();
      }
      static constexpr unsigned
      size_max()
      {
        return 0x7f000000u;
      }
      static constexpr unsigned
      seconds_max()
      {
        return std::chrono::duration_values<unsigned>::max();
      }
      static constexpr char const*
      granularity_default()
      {
        return "Event";
      }
    };

    struct Config : Defaults {
      using Name = fhicl::Name;
      using Comment = fhicl::Comment;
      template <typename T>
      using Atom = fhicl::Atom<T>;
      template <typename T>
      using OptionalAtom = fhicl::OptionalAtom<T>;
      Atom<unsigned> maxEvents{Name("maxEvents"), unsigned_max()};
      Atom<unsigned> maxSubRuns{Name("maxSubRuns"), unsigned_max()};
      Atom<unsigned> maxRuns{Name("maxRuns"), unsigned_max()};
      Atom<unsigned> maxInputFiles{Name("maxInputFiles"), unsigned_max()};
      Atom<unsigned> maxSize{Name("maxSize"),
                             Comment("Maximum size of file (in KiB)"),
                             size_max()};
      Atom<unsigned> maxAge{Name("maxAge"),
                            Comment("Maximum age of output file (in seconds)"),
                            seconds_max()};

      fhicl::Atom<std::string> granularity{
        fhicl::Name("granularity"),
        Comment(
          "The 'granularity' parameter specifies the level at which\n"
          "a file may be closed, and thereby the granularity of the file.\n"
          "The following values are possible:\n\n"
          "    Value        Meaning\n"
          "   =======================================================\n"
          "   \"Event\"       Allow file switch at next Event\n"
          "   \"SubRun\"      Allow file switch at next SubRun\n"
          "   \"Run\"         Allow file switch at next Run\n"
          "   \"InputFile\"   Allow file switch at next InputFile\n"
          "   \"Job\"         File closes at the end of Job\n\n"
          "For example, if a granularity of \"SubRun\" is specified, but the\n"
          "file has reached the maximum events written to disk (as specified\n"
          "by the 'maxEvents' parameter), switching to a new file will NOT\n"
          "happen until a new SubRun has been reached (or there are no more\n"
          "Events/SubRuns/Runs to process)."),
        granularity_default()};
    };

  public:
    ~ClosingCriteria();
    ClosingCriteria();
    ClosingCriteria(Config const& fp);
    ClosingCriteria(FileProperties const& fp, std::string const& granularity);
    FileProperties const& fileProperties() const;
    Granularity granularity() const;
    bool should_close(FileProperties const&) const;

  private:
    FileProperties closingCriteria_;
    Granularity granularity_{
      Granularity::value(ClosingCriteria::Defaults::granularity_default())};
  };
} // namespace art

#endif /* art_Framework_IO_ClosingCriteria_h */

// Local variables:
// mode: c++
// End:
