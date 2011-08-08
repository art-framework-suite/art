#ifndef art_Framework_Worker_ActionCodes_h
#define art_Framework_Worker_ActionCodes_h
namespace art {
  namespace actions {
    enum ActionCodes {
      IgnoreCompletely=0,
      Rethrow,
      SkipEvent,
      FailModule,
      FailPath,
      LastCode
    };
  }
}
#endif /* art_Framework_Worker_ActionCodes_h */

// Local Variables:
// mode: c++
// End:
