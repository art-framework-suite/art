#ifndef art_Framework_IO_PostCloseFileRenamer_h
#define art_Framework_IO_PostCloseFileRenamer_h

#include "boost/regex.hpp"
#include <string>

namespace art {
  class FileStatsCollector;
  class PostCloseFileRenamer;
} // namespace art

class art::PostCloseFileRenamer {
public:
  PostCloseFileRenamer(FileStatsCollector const& stats);

  // Apply substitutions according to given pattern from currently
  // collected stats.
  std::string applySubstitutions(std::string const& filePattern) const;

  // Rename file inPath according to pattern toPattern, returning
  // destination.
  std::string maybeRenameFile(std::string const& inPath,
                              std::string const& toPattern);

private:
  std::string subInputFileName_(boost::smatch const& match) const;
  std::string subTimestamp_(boost::smatch const& match) const;
  std::string subFilledNumeric_(boost::smatch const& match) const;

  FileStatsCollector const& stats_;
};

#endif /* art_Framework_IO_PostCloseFileRenamer_h */

// Local Variables:
// mode: c++
// End:
