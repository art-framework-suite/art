#ifndef art_Framework_Core_OutputFileStatus_h
#define art_Framework_Core_OutputFileStatus_h

#include "canvas/Utilities/Exception.h"
#include <ostream>

namespace art {
  enum class OutputFileStatus { Open, Switching, Closed };

  inline std::ostream&
  operator<<(std::ostream& os, OutputFileStatus const ofs)
  {
    switch (ofs) {
    case OutputFileStatus::Open:
      return os << "Open";
    case OutputFileStatus::Switching:
      return os << "Switching";
    case OutputFileStatus::Closed:
      return os << "Closed";
    default:
      throw art::Exception{art::errors::LogicError,
                           "Unknown output file status."};
    }
  }
} // namespace art

#endif /* art_Framework_Core_OutputFileStatus_h */

// Local variables:
// mode: c++
// End:
