#ifndef art_Utilities_Transition_h
#define art_Utilities_Transition_h
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

#endif /* art_Utilities_Transition_h */

// Local Variables:
// mode: c++
// End:
