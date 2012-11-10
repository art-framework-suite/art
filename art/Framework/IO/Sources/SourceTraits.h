#ifndef art_Framework_IO_Sources_ReaderTraits_h
#define art_Framework_IO_Sources_ReaderTraits_h
////////////////////////////////////////////////////////////////////////
// SourceTraits
//
// Attributes controlling the behavior of Source with respect to a
// particular detail class.
//
// Specialize as required.
//
////////////////////////////////////////////////////////////////////////

namespace art {
  // We are a generator, not a reader (or, we read our data from
  // somewhere other than files specified by source.fileNames).
  template <typename DETAIL>
  struct Source_generator {
    static constexpr bool value = false;
  };

  // Use the standard service interfaces (CatalogInterface and
  // FileTransfer) to obtain files.
  template <typename DETAIL>
  struct Source_wantFileServices {
    // If you're a generator, you almost certainly don't want to use the
    // standard file services.
    static constexpr bool value = !Source_generator<DETAIL>::value;
  };

}
#endif /* art_Framework_IO_Sources_ReaderTraits_h */

// Local Variables:
// mode: c++
// End:
