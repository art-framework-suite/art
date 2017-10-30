#include <ostream>

#include "TFile.h"
#include "TROOT.h"
#include "art/Framework/Services/Optional/TFileDirectory.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/exception.h"

using namespace std;

art::TFileDirectory::TFileDirectory(std::string const& dir,
                                    std::string const& descr,
                                    TFile* file,
                                    std::string const& path)
  : file_{file}, dir_{dir}, descr_{descr}, path_{path}
{}

void
art::TFileDirectory::invokeCallbacks()
{
  for (auto const& pr : callbacks_) {
    for (auto f : pr.second) {
      f();
    }
  }
}

void
art::TFileDirectory::registerCallback(Callback_t cb)
{
  callbacks_[fullPath()].push_back(cb);
}

void
art::TFileDirectory::cd() const
{
  auto const& fpath = fullPath();
  if (requireCallback_) {
    auto it = callbacks_.find(fpath);
    if (it == cend(callbacks_)) {
      throw Exception{errors::Configuration,
          "A TFileService error occured while attempting to make a directory or ROOT object.\n"}
      << "File-switching has been enabled for TFileService.  All modules must register\n"
      << "a callback function to be invoked whenever a file switch occurs.  The callback\n"
      << "must ensure that any pointers to ROOT objects have been updated.\n\n"
      << "  No callback has been registered for module '" << fpath << "'.\n\n"
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
    dir = dir->mkdir(dir_.c_str(), descr_.c_str());
    if (dir == nullptr) {
      throw cet::exception("InvalidDirectory")
        << "Can't create directory " << dir_ << " in path: " << path_;
    }
  }
  if (!file_->cd(fpath.c_str())) {
    throw cet::exception("InvalidDirectory")
      << "Can't change directory to path: " << fpath;
  }
}

std::string
art::TFileDirectory::fullPath() const
{
  return path_.empty() ? dir_ : path_ + "/" + dir_;
}

art::TFileDirectory
art::TFileDirectory::mkdir(std::string const& dir, std::string const& descr)
{
  detail::RootDirectorySentry sentry;
  cd();
  return TFileDirectory{dir, descr, file_, fullPath()};
}
