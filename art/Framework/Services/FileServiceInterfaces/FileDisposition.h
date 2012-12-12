#ifndef art_Framework_Services_FileServiceInterfaces_FileDisposition_h
#define art_Framework_Services_FileServiceInterfaces_FileDisposition_h

#include <string>

namespace art {
  enum class FileDisposition;

  // Translate enum to string;
  std::string translateFileDisposition(FileDisposition fd);

}

enum class art::FileDisposition {
  PENDING = -1,
    TRANSFERRED,
    CONSUMED,
    SKIPPED,
    INCOMPLETE
            };

#endif /* art_Framework_Services_FileServiceInterfaces_FileDisposition_h */

// Local Variables:
// mode: c++
// End:
