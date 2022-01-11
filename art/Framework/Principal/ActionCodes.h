#ifndef art_Framework_Principal_ActionCodes_h
#define art_Framework_Principal_ActionCodes_h
// vim: set sw=2 expandtab :

namespace art::actions {
  enum ActionCodes {
    IgnoreCompletely = 0,
    Rethrow,    // 1
    SkipEvent,  // 2
    FailModule, // 3
    FailPath,   // 4
    LastCode    // 5
  };
} // namespace art::actions

#endif /* art_Framework_Principal_ActionCodes_h */

// Local Variables:
// mode: c++
// End:
