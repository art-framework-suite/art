#ifndef art_Framework_IO_Root_rootErrMsgs_h
#define art_Framework_IO_Root_rootErrMsgs_h

#include <string>

namespace art {

  inline std::string
  couldNotFindTree(std::string const& treename)
  {
    std::string message = "Could not find tree ";
    message.append(treename);
    message.append(" in the input file.\n");
    return message;
  }

} // namespace art

#endif /* art_Framework_IO_Root_rootErrMsgs_h */

// Local Variables:
// mode: c++
// End:
