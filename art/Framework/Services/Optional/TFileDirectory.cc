#include <ostream>

#include "TFile.h"
#include "TROOT.h"
#include "art/Framework/Services/Optional/TFileDirectory.h"
#include "cetlib/exception.h"

using namespace std;

art::TFileDirectory::TFileDirectory(std::string const& dir,
                                    std::string const& descr,
                                    TFile* file,
                                    std::string const& path)
  : file_{file}, dir_{dir}, descr_{descr}, path_{path}
{}

void
art::TFileDirectory::cd() const
{
  auto const& fpath = fullPath();
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
