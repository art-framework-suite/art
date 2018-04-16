#ifndef art_Framework_Services_Optional_TFileDirectory_h
#define art_Framework_Services_Optional_TFileDirectory_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Optional/detail/RootDirectorySentry.h"
#include "hep_concurrency/RecursiveMutex.h"

#include "TDirectory.h"

#include <map>
#include <string>
#include <vector>

class TFile;
class TGraph;

#include <functional>
#include <map>
#include <string>
#include <utility>

namespace art {

  // There are no public constructors, so can only be made by derived classes.
  class TFileDirectory {
    // Types -- For derived classes
  protected:
    using Callback_t = std::function<void()>;
    // Special Member Functions
  public:
    virtual ~TFileDirectory();
    TFileDirectory(TFileDirectory const&);
    TFileDirectory(TFileDirectory&&);
    TFileDirectory& operator=(TFileDirectory const&) = delete;
    TFileDirectory& operator=(TFileDirectory&&) = delete;
    // Special Member Functions -- For derived classes
  protected:
    TFileDirectory(std::string const& dir,
                   std::string const& descr,
                   TFile* file,
                   std::string const& path);
    // API -- For users
  public:
    // Make new ROOT object of type T using args. It will be made in the current
    // directory, as established with a call to cd.
    template <typename T, typename... ARGS>
    T* make(ARGS... args) const;
    // Make and register a new ROOT object of type T, giving it the specified
    // name and title, using args. The type must be registerable, and must
    // support naming and titling.
    template <typename T, typename... ARGS>
    T* makeAndRegister(char const* name, char const* title, ARGS... args) const;
    template <typename T, typename... ARGS>
    T* makeAndRegister(std::string const& name,
                       std::string const& title,
                       ARGS... args) const;
    // Create a new TFileDirectory, sharing the same TFile as this one, but with
    // an additional dir, and with path being the absolute path of this one.
    TFileDirectory mkdir(std::string const& dir, std::string const& descr = "");
    // API -- For derived classes
  protected:
    // Return the full pathname of represented directory, that is path_ + dir_.
    std::string fullPath() const;
    void registerCallback(Callback_t);
    void invokeCallbacks();
    // Member Data -- For derived classes
  protected:
    // Protects all data members, including derived classes.
    mutable hep::concurrency::RecursiveMutex mutex_{
      "art::TFileDirectory::mutex_"};
    // The root file.
    TFile* file_;
    // Directory name in the root file.
    std::string dir_;
    // Directory title in the root file.
    std::string descr_;
    // Callbacks must exist for each directory. Used only by TFileService.
    bool requireCallback_{false};
    // API -- Implementation details
  private:
    // Make the current directory be the one implied by the state of this
    // TFileDirectory.
    void cd() const;
    // Member Data -- My implementation details.
  private:
    std::string path_;
    std::map<std::string, std::vector<Callback_t>> callbacks_;
  };

  template <typename T, typename... ARGS>
  T*
  TFileDirectory::make(ARGS... args) const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    detail::RootDirectorySentry rds;
    cd();
    auto ret = new T{args...};
    return ret;
  }

  template <typename T, typename... ARGS>
  T*
  TFileDirectory::makeAndRegister(char const* name,
                                  char const* title,
                                  ARGS... args) const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    detail::RootDirectorySentry rds;
    cd();
    auto ret = new T{args...};
    ret->SetName(name);
    ret->SetTitle(title);
    gDirectory->Append(ret);
    return ret;
  }

  template <typename T, typename... ARGS>
  T*
  TFileDirectory::makeAndRegister(std::string const& name,
                                  std::string const& title,
                                  ARGS... args) const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    auto ret = makeAndRegister(name.c_str(), title.c_str(), args...);
    return ret;
  }

} // namespace art

#endif /* art_Framework_Services_Optional_TFileDirectory_h */

// Local Variables: -
// mode: c++ -
// c-basic-offset: 2 -
// indent-tabs-mode: nil -
// End: -
