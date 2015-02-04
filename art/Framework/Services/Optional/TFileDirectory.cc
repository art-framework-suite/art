#include <ostream>

#include "TROOT.h"
#include "TFile.h"
#include "art/Framework/Services/Optional/TFileDirectory.h"
#include "cetlib/exception.h"

using namespace std;

namespace art
{

  TFileDirectory::TFileDirectory( const std::string & dir, const std::string & descr,
				  TFile * file, const std::string & path ) :
    file_( file ), dir_( dir ), descr_( descr ), path_( path )
  { }

  void TFileDirectory::cd() const {
    string fpath = fullPath();
    TDirectory * dir = file_->GetDirectory( fpath.c_str() );
    if ( dir == nullptr ) {
      if ( ! path_.empty() ) {
        dir = file_->GetDirectory( path_.c_str() );
        if ( dir == nullptr )
          throw
            cet::exception( "InvalidDirectory" )
            << "Can't change directory to path: " << path_;
      } else {
        dir = file_;
      }
      dir = dir->mkdir( dir_.c_str(), descr_.c_str() );
      if ( dir == nullptr )
        throw
          cet::exception( "InvalidDirectory" )
          << "Can't create directory " << dir_ << " in path: " << path_;
    }
    bool ok = file_->cd( fpath.c_str() );
    if ( ! ok )
      throw
        cet::exception( "InvalidDirectory" )
        << "Can't change directory to path: " << fpath;
  }

  std::string TFileDirectory::fullPath() const {
    return path_.empty() ? dir_ : path_ + "/" + dir_;
  }

  TFileDirectory TFileDirectory::mkdir( const std::string & dir, const std::string & descr ) {
    RootDirectorySentry sentry;
    cd();
    return TFileDirectory( dir, descr, file_, fullPath() );
  }
}
