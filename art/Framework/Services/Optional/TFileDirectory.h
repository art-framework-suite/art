#ifndef art_Framework_Services_Optional_TFileDirectory_h
#define art_Framework_Services_Optional_TFileDirectory_h

/* class TFileDirectory
 *
 * Original author: Luca Lista, INFN
 *
 */

#include <string>
#include <utility>

#include "art/Framework/Services/Optional/detail/TH1AddDirectorySentry.h"

class TFile;
class TGraph;

#include "TDirectory.h"

namespace art
{
  class TFileDirectory {
  public:

    virtual ~TFileDirectory() = default;

    /// make new ROOT object of type T, using constructor parameters
    /// args. It will be made in the 'current directory', as
    /// established with a call to 'cd'.
    template <typename T, typename ... ARGS>
    T* make(ARGS ... args) const;

    /// make and register a new ROOT object of type T, giving it the
    /// specified name and title, using constructor parameters
    /// 'args'. The type must be registerable, and must support naming
    /// and titling.
    template <typename T, typename ... ARGS>
    T* makeAndRegister(char const* name, const char* title, ARGS ... args) const;

    template <typename T, typename ... ARGS>
    T* makeAndRegister(std::string const& name, std::string const& title, ARGS ... args) const;

    /// Create a new TFileDirectory, sharing the same TFile as this
    /// one, but with an additional 'dir', and with 'path' being the
    /// fullPath() of this one.
    TFileDirectory mkdir( const std::string & dir, const std::string & descr = "" );

  protected:
    /// Create a new TFileDirectory object.
    TFileDirectory( const std::string & dir, const std::string & descr,
                    TFile * file, const std::string & path );

    /// Return the full pathname of the current directory, formed from
    /// appending 'dir' to the end of 'path'.
    std::string fullPath() const;

    TFile * file_;
    std::string dir_;
    std::string descr_;
    std::string path_;

  private:
    /// Make the current directory be the one implied by the state of
    /// this TFileDirectory.
    void cd() const;
  };

  template <typename T, typename ... ARGS>
  T*
  TFileDirectory::make(ARGS ... args) const
  {
    RootDirectorySentry sentry;
    cd();
    return new T(std::forward<ARGS>(args)...);
  }

  template <typename T, typename ... ARGS>
  T*
  TFileDirectory::makeAndRegister(char const* name,
                                  char const* title,
                                  ARGS ... args) const
  {
    RootDirectorySentry sentry;
    cd();
    T* p = new T(std::forward<ARGS>(args)...);
    p->SetName(name);
    p->SetTitle(title);
    gDirectory->Append(p);
    return p;
  }

  template <typename T, typename ... ARGS>
  T*
  TFileDirectory::makeAndRegister(std::string const& name,
                                  std::string const& title,
                                  ARGS ... args) const
  {
    return makeAndRegister(name.c_str(),
                           title.c_str(),
                           std::forward<ARGS>(args)...);
  }

}

#endif /* art_Framework_Services_Optional_TFileDirectory_h */

/// emacs configuration

/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
