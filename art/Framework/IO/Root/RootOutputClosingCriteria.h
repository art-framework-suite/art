#ifndef art_Framework_IO_Root_RootOutputClosingCriteria_h
#define art_Framework_IO_Root_RootOutputClosingCriteria_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/OutputFileGranularity.h"
#include "art/Framework/Core/OutputFileStatus.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/FileIndex.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalAtom.h"

#include <chrono>
#include <type_traits>

namespace art {

  class FileProperties {

  public:
    ~FileProperties();

    FileProperties();

    FileProperties(unsigned events,
                   unsigned subRuns,
                   unsigned runs,
                   unsigned inputFiles,
                   unsigned size,
                   std::chrono::seconds age);

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

    // template <Granularity::BT B>
    // std::enable_if_t < B != Granularity::InputFile >
    // update(OutputFileStatus const status)
    //{
    //  ++treeEntryNumbers_[B];
    //  if (status != OutputFileStatus::Switching) {
    //    ++counts_[B];
    //  }
    //}

    // template <Granularity::BT B>
    // std::enable_if_t<B == Granularity::InputFile>
    // update()
    //{
    //  ++counts_[B];
    //}

    void updateSize(unsigned const size);

    void updateAge(std::chrono::seconds const age);

  private:
    std::array<unsigned, Granularity::NBoundaries()> counts_{{}};

    std::array<FileIndex::EntryNumber_t, Granularity::NBoundaries() - 1>
      treeEntryNumbers_{{}};

    std::chrono::seconds age_{std::chrono::seconds::zero()};

    unsigned size_{};
  };

  // template <Granularity::BT B>
  // std::enable_if_t < B != Granularity::InputFile >
  // FileProperties::
  // update(OutputFileStatus const status)
  //{
  //  ++treeEntryNumbers_[B];
  //  if (status != OutputFileStatus::Switching) {
  //    ++counts_[B];
  //  }
  //}

  // template <Granularity::BT B>
  // std::enable_if_t<B == Granularity::InputFile>
  // FileProperties::
  // update()
  //{
  //  ++counts_[B];
  //}

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
          "an output file may be closed, and thereby the granularity\n"
          "of the file.  The following values are possible:\n\n"
          "    Value        Meaning\n"
          "   =======================================================\n"
          "   \"Event\"       Allow file switch at next Event\n"
          "   \"SubRun\"      Allow file switch at next SubRun\n"
          "   \"Run\"         Allow file switch at next Run\n"
          "   \"InputFile\"   Allow file switch at next InputFile\n"
          "   \"Job\"         File closes at the end of Job\n\n"
          "For example, if a granularity of \"SubRun\" is specified, but "
          "the\n"
          "output-module has reached the maximum events written to disk (as\n"
          "specified by the 'maxEvents' parameter), the output module will "
          "NOT\n"
          "switch to a new file until a new SubRun has been reached (or\n"
          "there are no more Events/SubRuns/Runs to process)."),
        granularity_default()};
    };

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

#endif /* art_Framework_IO_Root_RootOutputClosingCriteria_h */

// Local variables:
// mode: c++
// End:
