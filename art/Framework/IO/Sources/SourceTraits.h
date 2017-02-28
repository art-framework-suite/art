#ifndef art_Framework_IO_Sources_SourceTraits_h
#define art_Framework_IO_Sources_SourceTraits_h
////////////////////////////////////////////////////////////////////////
// SourceTraits
//
// Attributes controlling the behavior of Source with respect to a
// particular detail class.
//
// Specialize as required.
//
// Generally speaking, type traits are simple struct or class templates
// whose purpose is to signal (at compile time) one or more properties
// of another type, specified as the (usually only) template
// argument. The property to be signalled may be a compile-time
// expression in the template, set explicitly, or (more rarely) left
// unspecified. One is expected to specialize the trait for your own
// types in order to convey the intended information about them.
////////////////////////////////////
// Available traits.
//
// Specialize the following traits for your source detail class if you
// wish to tune the behavior of your InputSource:
//
// 1. Source_generator.
//
// If art::Source_generator<MyClass>::value is true, it indicates that
// your source detail class MyClass does not read files or otherwise use
// the input string provided to it in its readFile(...)
// function. Defaults to false; specialize to true for your detail class
// XXXX as below:
//
// namespace art {
//   template<>
//     struct Source_generator<XXXX> {
//       static constexpr bool value = true;
//     };
// }
//
// 2. Source_wantFileServices.
//
// If art::Source_wantFileServices<MyClass>::value is false you are
// representing that your source detail class MyClass will be working
// from the list of filenames provided by the FHiCL parameter
// source.fileNames, but does not want them treated as real files by the
// standard service interfaces (CatalogInterface and FileTransfer). Use
// this if source.fileNames is a list of URLS for streaming data, for
// instance. Absent specialization,
// art::Source_wantFileServices<MyClass>::value is defined as the
// logical negation of art::Source_generator<MyClass>::value (see
// above); specialize as you wish.
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
#endif /* art_Framework_IO_Sources_SourceTraits_h */

// Local Variables:
// mode: c++
// End:
