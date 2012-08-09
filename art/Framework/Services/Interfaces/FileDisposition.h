#ifndef art_Framework_Services_Interfaces_FileDisposition_h
#define art_Framework_Services_Interfaces_FileDisposition_h

namespace art {
  enum class FileDisposition;
}

enum class art::FileDisposition {
  TRANSFERRED,
  CONSUMED,
  SKIPPED
};

#endif /* art_Framework_Services_Interfaces_FileDisposition_h */

// Local Variables:
// mode: c++
// End:
