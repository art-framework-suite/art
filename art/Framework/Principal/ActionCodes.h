#ifndef art_Framework_Principal_ActionCodes_h
#define art_Framework_Principal_ActionCodes_h
// vim: set sw=2 expandtab :

namespace art {
namespace actions {

enum ActionCodes {
  IgnoreCompletely, // 0
  Rethrow, // 1
  SkipEvent, // 2
  FailModule, // 3
  FailPath, // 4
  LastCode // 5
};

} // namespace actions
} // namespace art

#endif /* art_Framework_Principal_ActionCodes_h */

// Local Variables:
// mode: c++
// End:
