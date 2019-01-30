#ifndef art_Framework_IO_detail_FileNameComponents_h
#define art_Framework_IO_detail_FileNameComponents_h

//=====================================================================
// The FileNameComponents class breaks the following type of pattern:
//
//   "run1_aladdin_%#_and_jasmine_%03#.root"
//
// into the components:
//
//   ("run1_aladdin_", "")
//   ("_and_jasmine_", "03")
//
// with the suffix ".root".  With these components and suffix, the
// following filename is assembled whenever a call of (e.g.)
// fileNameWithIndex(4) is made:
//
//   "run1_aladdin_4_and_jasmine_004.root"
//
//=====================================================================

#include <string>
#include <utility>
#include <vector>

namespace art::detail {

  class FileNameComponents {
  public:
    void add(std::string const& prefix, std::string const& digitFormat);

    void setSuffix(std::string suffix);

    std::string fileNameWithIndex(std::size_t index) const;

    bool operator<(FileNameComponents const& fnc) const;

  private:
    std::vector<std::pair<std::string, std::string>> components_;
    std::string suffix_{};
  };

  FileNameComponents componentsFromPattern(std::string const& pattern);
}

#endif /* art_Framework_IO_detail_FileNameComponents_h */

// Local Variables:
// mode: c++
// End:
