#ifndef art_Utilities_Verbosity_h
#define art_Utilities_Verbosity_h
// A first attempt to define a descriptive enumeration for verbosity.
namespace art {
  enum Verbosity {
    Silent=0,
    Concise=2,
    Normal=5,
    Detailed=10
  };
}
#endif /* art_Utilities_Verbosity_h */

// Local Variables:
// mode: c++
// End:
