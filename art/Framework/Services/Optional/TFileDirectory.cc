#include "art/Framework/Services/Optional/TFileDirectory.h"
// vim: set sw=2 expandtab :

#include "canvas/Utilities/Exception.h"
#include "cetlib_except/exception.h"
#include "hep_concurrency/RecursiveMutex.h"

#include "TFile.h"
#include "TROOT.h"

#include <map>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

using namespace std;
using namespace std::string_literals;
using namespace hep::concurrency;

namespace art {

  TFileDirectory::~TFileDirectory() {}

  TFileDirectory::TFileDirectory(string const& dir,
                                 string const& descr,
                                 TFile* file,
                                 string const& path)
    : file_{file}, dir_{dir}, descr_{descr}, path_{path}
  {}

  TFileDirectory::TFileDirectory(TFileDirectory const& rhs)
    : mutex_{"art::TFileDirectory::mutex_"}
    , file_{rhs.file_}
    , dir_{rhs.dir_}
    , descr_{rhs.descr_}
    , requireCallback_{rhs.requireCallback_}
    , path_{rhs.path_}
    , callbacks_{rhs.callbacks_}
  {}

  TFileDirectory::TFileDirectory(TFileDirectory&& rhs)
    : mutex_{"art::TFileDirectory::mutex_"}
    , file_{move(rhs.file_)}
    , dir_{move(rhs.dir_)}
    , descr_{move(rhs.descr_)}
    , requireCallback_{move(rhs.requireCallback_)}
    , path_{move(rhs.path_)}
    , callbacks_{move(rhs.callbacks_)}
  {}

  string
  TFileDirectory::fullPath() const
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    string ret;
    if (path_.empty()) {
      ret = dir_;
    } else {
      ret = path_ + "/"s + dir_;
    }
    return ret;
  }

  void
  TFileDirectory::cd() const
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    auto const fpath = fullPath();
    if (requireCallback_) {
      auto iter = callbacks_.find(dir_);
      if (iter == callbacks_.end()) {
        throw Exception{errors::Configuration,
                        "A TFileService error occured while attempting to make "
                        "a directory or ROOT object.\n"}
          << "File-switching has been enabled for TFileService.  All modules "
             "must register\n"
          << "a callback function to be invoked whenever a file switch occurs. "
             " The callback\n"
          << "must ensure that any pointers to ROOT objects have been "
             "updated.\n\n"
          << "  No callback has been registered for directory '" << dir_
          << "'.\n\n"
          << "Contact artists@fnal.gov for guidance.";
      }
    }
    TDirectory* dir = file_->GetDirectory(fpath.c_str());
    if (dir == nullptr) {
      if (!path_.empty()) {
        dir = file_->GetDirectory(path_.c_str());
        if (dir == nullptr) {
          throw cet::exception("InvalidDirectory")
            << "Can't change directory to path: " << path_;
        }
      } else {
        dir = file_;
      }
      auto newdir = dir->mkdir(dir_.c_str(), descr_.c_str());
      if (newdir == nullptr) {
        throw cet::exception("InvalidDirectory")
          << "Can't create directory " << dir_ << " in path: " << path_;
      }
    }
    auto ok = file_->cd(fpath.c_str());
    if (!ok) {
      throw cet::exception("InvalidDirectory")
        << "Can't change directory to path: " << fpath;
    }
  }

  TFileDirectory
  TFileDirectory::mkdir(string const& dir, string const& descr)
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    detail::RootDirectorySentry rds;
    cd();
    return TFileDirectory{dir, descr, file_, fullPath()};
  }

  void
  TFileDirectory::invokeCallbacks()
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    for (auto const& dirAndvcallback : callbacks_) {
      dir_ = dirAndvcallback.first;
      for (auto cb : dirAndvcallback.second) {
        cb();
      }
    }
  }

  void
  TFileDirectory::registerCallback(Callback_t cb)
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    callbacks_[dir_].push_back(cb);
  }

} // namespace art
