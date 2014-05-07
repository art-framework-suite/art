#ifndef art_Framework_Core_OutputFileInfo_h
#define art_Framework_Core_OutputFileInfo_h

#include <string>

namespace art {
  class OutputFileInfo;
}

class art::OutputFileInfo {
public:
  OutputFileInfo(std::string const & moduleLabel,
                 std::string const & fileName);
  std::string const & moduleLabel() const;
  std::string const & fileName() const;

private:
  std::string const & moduleLabel_;
  std::string const & fileName_;
};

#ifndef __GCXML__
inline
art::OutputFileInfo::
OutputFileInfo(std::string const & moduleLabel,
               std::string const & fileName)
  :
  moduleLabel_(moduleLabel),
  fileName_(fileName)
{
}

inline
std::string const &
art::OutputFileInfo::
moduleLabel() const
{
  return moduleLabel_;
}

inline
std::string const &
art::OutputFileInfo::
fileName() const
{
  return fileName_;
}
#endif /* __GCCXML__ */

#endif /* art_Framework_Core_OutputFileInfo_h */

// Local Variables:
// mode: c++
// End:
