#ifndef art_Utilities_Transitions_h
#define art_Utilities_Transitions_h
// vim: set sw=2 expandtab :

namespace art {

enum class Transition {
    BeginJob
  , EndJob
  , BeginFile
  , EndFile
  , BeginRun
  , EndRun
  , BeginSubRun
  , EndSubRun
  , BeginEvent
  , EndEvent
};

} // namespace art

#endif /* art_Utilities_Transitions_h */

// Local Variables:
// mode: c++
// End:
