#ifndef art_Framework_IO_Root_RootOutputClosingCriteria_h
#define art_Framework_IO_Root_RootOutputClosingCriteria_h
// vim: set sw=2:

#include "art/Framework/Core/OutputFileGranularity.h"
#include "art/Framework/Core/OutputFileStatus.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/FileIndex.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalAtom.h"

#include <chrono>
#include <type_traits>

#define GRANULARITY_COMMENT                                                    \
  "The 'granularity' parameter specifies the level at which\n"                 \
  "an output file may be closed, and thereby the granularity\n"                \
  "of the file.  The following values are possible:\n\n"                       \
  "    Value        Meaning\n"                                                 \
  "   =======================================================\n"               \
  "   \"Event\"       Allow file switch at next Event\n"                       \
  "   \"SubRun\"      Allow file switch at next SubRun\n"                      \
  "   \"Run\"         Allow file switch at next Run\n"                         \
  "   \"InputFile\"   Allow file switch at next InputFile\n"                   \
  "   \"Job\"         File closes at the end of Job\n\n"                       \
  "For example, if a granularity of \"SubRun\" is specified, but the\n"        \
  "output-module has reached the maximum events written to disk (as\n"         \
  "specified by the 'maxEvents' parameter), the output module will NOT\n"      \
  "switch to a new file until a new SubRun has been reached (or\n"             \
  "there are no more Events/SubRuns/Runs to process)."

namespace art {

  struct Defaults {
    static constexpr auto
    unsigned_max()
    {
      return std::numeric_limits<unsigned>::max();
    }
    static constexpr auto
    size_max()
    {
      return 0x7f000000u;
    }
    static constexpr auto
    seconds_max()
    {
      return std::chrono::duration_values<unsigned>::max();
    }
    static constexpr auto
    granularity_default()
    {
      return "Event";
    }
  };

  class FileProperties {
  public:
    FileProperties() = default;
    FileProperties(unsigned events,
                   unsigned subRuns,
                   unsigned runs,
                   unsigned inputFiles,
                   unsigned size,
                   std::chrono::seconds age);

    auto
    nEvents() const
    {
      return counts_[Granularity::Event];
    }
    auto
    nSubRuns() const
    {
      return counts_[Granularity::SubRun];
    }
    auto
    nRuns() const
    {
      return counts_[Granularity::Run];
    }
    auto
    nInputFiles() const
    {
      return counts_[Granularity::InputFile];
    }
    auto
    size() const
    {
      return size_;
    }
    auto
    age() const
    {
      return age_;
    }

    auto
    eventEntryNumber() const
    {
      return treeEntryNumbers_[Granularity::Event];
    }
    auto
    subRunEntryNumber() const
    {
      return treeEntryNumbers_[Granularity::SubRun];
    }
    auto
    runEntryNumber() const
    {
      return treeEntryNumbers_[Granularity::Run];
    }

    template <Granularity::BT B>
    std::enable_if_t<B != Granularity::InputFile>
    update(OutputFileStatus const status)
    {
      ++treeEntryNumbers_[B];
      if (status != OutputFileStatus::Switching) {
        ++counts_[B];
      }
    }

    template <Granularity::BT B>
    std::enable_if_t<B == Granularity::InputFile>
    update()
    {
      ++counts_[B];
    }

    void
    updateSize(unsigned const size)
    {
      size_ = size;
    }
    void
    updateAge(std::chrono::seconds const age)
    {
      age_ = age;
    }

  private:
    std::array<unsigned, Granularity::NBoundaries()> counts_{
      {}}; // Filled by aggregation
    std::array<FileIndex::EntryNumber_t, Granularity::NBoundaries() - 1>
      treeEntryNumbers_{{}};
    std::chrono::seconds age_{std::chrono::seconds::zero()};
    unsigned size_{};
  };

  std::ostream& operator<<(std::ostream& os, FileProperties const& fp);

  class ClosingCriteria {
  public:
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
      fhicl::Atom<std::string> granularity{fhicl::Name("granularity"),
                                           Comment(GRANULARITY_COMMENT),
                                           granularity_default()};
    };

    ClosingCriteria(Config const& fp);
    ClosingCriteria(FileProperties const& fp, std::string const& granularity);
    ClosingCriteria() = default;

    auto const&
    fileProperties() const
    {
      return closingCriteria_;
    }
    auto
    granularity() const
    {
      return granularity_;
    }

    bool should_close(FileProperties const&) const;

  private:
    FileProperties closingCriteria_;
    Granularity granularity_{
      Granularity::value(Defaults::granularity_default())};
  };
}

#undef GRANULARITY_COMMENT
#endif /* art_Framework_IO_Root_RootOutputClosingCriteria_h */

// Local variables:
// mode: c++
// End:
