#ifndef UtilAlgos_TFileService_h
#define UtilAlgos_TFileService_h


/* \class TFileService
 *
 * \author Luca Lista, INFN
 *
 */


#include "art/Framework/Core/TFileDirectory.h"
#include "fhicl/ParameterSet.h"


namespace edm {
  class ActivityRegistry;
  class ModuleDescription;

  class TFileService : public TFileDirectory {
  public:
    /// constructor
    TFileService(const fhicl::ParameterSet &, edm::ActivityRegistry &);
    /// destructor
    ~TFileService();
    /// return opened TFile
    TFile & file() const { return * file_; }

  private:
    /// pointer to opened TFile
    TFile * file_;
    std::string fileName_;
    bool fileNameRecorded_;
    bool closeFileFast_;
    // set current directory according to module name and prepare to create directory
    void setDirectoryName( const edm::ModuleDescription & desc );
  };

}  // namespace edm

#endif  // UtilAlgos_TFileService_h
