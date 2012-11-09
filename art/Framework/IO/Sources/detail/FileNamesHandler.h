#ifndef art_Framework_IO_Sources_detail_FileNamesHandler_h
#define art_Framework_IO_Sources_detail_FileNamesHandler_h

#include "art/Framework/IO/Sources/detail/FileServiceProxy.h"

#include <string>
#include <vector>

namespace art {
  namespace detail {

    template <bool wantFileServices>
    class FileNamesHandler;

    // Handle files when we want to use the official services.
    template <>
    class FileNamesHandler<true> {
    public:
      explicit FileNamesHandler(std::vector<std::string> && fileNames,
                                size_t attempts = 5,
                                double waitBetweenAttempts = 5.0);

      std::string next();

    private:
      FileServiceProxy fp_;
    };

    // Handle files when we don't.
    template <>
    class FileNamesHandler<false> {
    public:
      explicit FileNamesHandler(std::vector<std::string> && fileNames,
                                size_t = 0);
      std::string next();

    private:
      std::vector<std::string> fileNames_;
      std::vector<std::string>::const_iterator currentFile_;
      std::vector<std::string>::const_iterator end_;
    };
  } // detail
} // art

art::detail::FileNamesHandler<true>::
FileNamesHandler(std::vector<std::string> && fileNames,
                 size_t attempts,
                 double waitBetweenAttempts)
  :
  fp_(std::move(fileNames),
      attempts,
      waitBetweenAttempts)
{
}

std::string
art::detail::FileNamesHandler<true>::
next()
{
  return fp_.next();
}

art::detail::FileNamesHandler<false>::
FileNamesHandler(std::vector<std::string> && fileNames, size_t)
  :
  fileNames_(std::move(fileNames)),
  currentFile_(fileNames_.begin()),
  end_(fileNames_.end())
{
}

std::string
art::detail::FileNamesHandler<false>::
next()
{
  return (currentFile_ == end_) ? std::string() : *(currentFile_++);
}
#endif /* art_Framework_IO_Sources_detail_FileNamesHandler_h */

// Local Variables:
// mode: c++
// End:
