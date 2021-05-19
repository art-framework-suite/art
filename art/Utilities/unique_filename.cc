#include "art/Utilities/unique_filename.h"

#include "canvas/Utilities/Exception.h"

#include "boost/filesystem.hpp"
#include "boost/system/error_code.hpp"

#include <cerrno>
#include <cstring>
#include <unistd.h>

extern "C" {
#include <fcntl.h>
#include <sys/stat.h>
}

std::string
art::unique_filename(std::string stem, std::string extension)
{
  boost::filesystem::path const p(stem + "-%%%%-%%%%-%%%%-%%%%" + extension);
  boost::filesystem::path outpath;
  boost::system::error_code ec;
  int tmp_fd = -1, error = 0;
  do {
    outpath = boost::filesystem::unique_path(p, ec);
  } while (!ec && (tmp_fd = creat(outpath.c_str(), S_IRUSR | S_IWUSR)) == -1 &&
           (error = errno) == EEXIST);
  if (tmp_fd != -1) {
    close(tmp_fd);
  } else {
    art::Exception e(art::errors::FileOpenError);
    e << "RootOutput cannot ascertain a unique temporary filename for output "
         "based on stem\n\""
      << stem << "\": ";
    if (ec) {
      e << ec;
    } else {
      e << strerror(error);
    }
    e << ".\n";
    throw e;
  }
  return outpath.native();
}
