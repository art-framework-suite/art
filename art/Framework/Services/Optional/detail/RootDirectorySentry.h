#ifndef art_Framework_Services_Optional_detail_RootDirectorySentry_h
#define art_Framework_Services_Optional_detail_RootDirectorySentry_h
// vim: set sw=2 expandtab :

//=================================================================
// RootDirectorySentry
//
// Description: Manages the status of the ROOT directory
//
// Usage: Construct an instance of this object in a routine in which you
//    expect a ROOT histogram to be automatically added to the current
//    directory in a file. The destructor will be sure to reset ROOT to
//    its previous setting.
//=================================================================

namespace art {
  namespace detail {

    class RootDirectorySentry {
    public:
      ~RootDirectorySentry() noexcept(false);
      RootDirectorySentry();
      RootDirectorySentry(RootDirectorySentry const&) = delete;
      RootDirectorySentry(RootDirectorySentry&&) = delete;
      RootDirectorySentry& operator=(RootDirectorySentry const&) = delete;
      RootDirectorySentry& operator=(RootDirectorySentry&&) = delete;

    private:
      bool status_;
    };

  } // namespace detail
} // namespace art

#endif /* art_Framework_Services_Optional_detail_RootDirectorySentry_h */

/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
