#ifndef art_Framework_IO_Sources_ReaderTraits_h
#define art_Framework_IO_Sources_ReaderTraits_h
////////////////////////////////////////////////////////////////////////
// ReaderTraits
//
// Attributes controlling the behavior of ReaderSource with respect to a
// particular detail class.
//
// Specialize as required.
//
////////////////////////////////////////////////////////////////////////
namespace art {
  template <typename DETAIL>
    struct Reader_wantFileServices {
      static constexpr bool value = true;
    };
}
#endif /* art_Framework_IO_Sources_ReaderTraits_h */

// Local Variables:
// mode: c++
// End:
