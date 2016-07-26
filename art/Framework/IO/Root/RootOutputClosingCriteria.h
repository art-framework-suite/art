#ifndef art_Framework_IO_Root_RootOutputClosingCriteria_h
#define art_Framework_IO_Root_RootOutputClosingCriteria_h
// vim: set sw=2:

#include "art/Framework/Core/OutputFileSwitchBoundary.h"
#include "canvas/Persistency/Provenance/FileIndex.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/OptionalAtom.h"

#include <chrono>
#include <type_traits>

#define GRANULARITY_COMMENT                                             \
  "The 'granularity' parameter specifies the level at which\n"          \
  "an output file may be closed, and thereby the granularity\n"         \
  "of the file.  The following values are possible:\n\n"               \
  "    Value        Meaning\n"                                          \
  "   =======================================================\n"        \
  "   \"Event\"       Allow file switch at next Event\n"                \
  "   \"SubRun\"      Allow file switch at next SubRun\n"               \
  "   \"Run\"         Allow file switch at next Run\n"                  \
  "   \"InputFile\"   Allow file switch at next InputFile\n\n"          \
  "For example, if a granularity of \"SubRun\" is specified, but the\n" \
  "output-module has reached the maximum events written to disk (as\n"  \
  "specified by the 'maxEvents' parameter), the output module will NOT\n" \
  "switch to a new file until a new SubRun has been reached (or\n" \
  "there are no more Events/SubRuns/Runs to process)."

namespace art {

  struct FileProperties {

    FileProperties() = default;
    FileProperties(unsigned events,
                   unsigned subRuns,
                   unsigned runs,
                   unsigned inputFiles,
                   unsigned size,
                   std::chrono::seconds age);

    unsigned nEvents {};
    unsigned nSubRuns {};
    unsigned nRuns {};
    unsigned nInputFiles {};
    unsigned size {};
    std::chrono::seconds age {std::chrono::seconds::zero()};
  };

  std::ostream& operator<<(std::ostream& os, FileProperties const& fp);

  class ClosingCriteria {
  public:

    struct Config {
      using Name = fhicl::Name;
      using Comment = fhicl::Comment;
      template <typename T> using Atom = fhicl::Atom<T>;
      template <typename T> using OptionalAtom = fhicl::OptionalAtom<T>;

      Atom<unsigned> maxEvents { Name("maxEvents"), -1u };
      Atom<unsigned> maxSubRuns { Name("maxSubRuns"), -1u };
      Atom<unsigned> maxRuns { Name("maxRuns"), -1u };
      Atom<unsigned> maxInputFiles { Name("maxInputFiles"), -1u };
      Atom<unsigned> maxSize { Name("maxSize"), Comment("Maximum size of file (in KiB)"), 0x7f000000u };
      Atom<unsigned> maxAge { Name("maxAge"), Comment("Maximum age of output file (in seconds)"), std::chrono::duration_values<unsigned>::max() };
      fhicl::Atom<std::string> granularity { fhicl::Name("granularity"), Comment(GRANULARITY_COMMENT), "Event" };
    };

    ClosingCriteria(Config const& fp);
    ClosingCriteria(FileProperties const& fp, std::string const& granularity);
    ClosingCriteria() = default;

    auto const& fileProperties() const { return closingCriteria_; }
    auto granularity() const { return granularity_; }

    bool should_close(FileProperties const&) const;

  private:
    FileProperties closingCriteria_;
    Boundary granularity_;
  };

}

#undef GRANULARITY_COMMENT
#endif /* art_Framework_IO_Root_RootOutputClosingCritieria_h */

// Local variables:
// mode: c++
// End:
