#ifndef art_Framework_Core_OutputFileStatus_h
#define art_Framework_Core_OutputFileStatus_h

namespace art {
  enum class OutputFileStatus { Open, StagedToSwitch, Switching, Closed };
}

#endif

// Local variables:
// mode: c++
// End:
