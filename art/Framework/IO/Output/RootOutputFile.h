#ifndef FRAMEWORK_IO_OUTPUT_ROOTOUTPUTFILE_H
#define FRAMEWORK_IO_OUTPUT_ROOTOUTPUTFILE_H

// ======================================================================
//
// RootOutputFile
//
// ======================================================================

#include "TROOT.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/IO/Output/RootOutput.h"
#include "art/Framework/IO/Output/RootOutputTree.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/FileID.h"
#include "art/Persistency/Provenance/FileIndex.h"
#include "art/Persistency/Provenance/ParameterSetBlob.h"
#include "art/Persistency/Provenance/ParameterSetMap.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/Selections.h"
#include "boost/array.hpp"
#include "boost/shared_ptr.hpp"
#include <map>
#include <string>
#include <vector>

// ----------------------------------------------------------------------

class TFile;
class TTree;

namespace art {
  class RootOutput;
  class History;

  class RootOutputFile {
  public:
    typedef RootOutput::OutputItem OutputItem;
    typedef RootOutput::OutputItemList OutputItemList;
    typedef boost::array<RootOutputTree *, NumBranchTypes> RootOutputTreePtrArray;
    explicit RootOutputFile(RootOutput * om, std::string const& fileName,
                            std::string const& logicalFileName);
    ~RootOutputFile() {}
    void writeOne(EventPrincipal const& e);
    //void endFile();
    void writeSubRun(SubRunPrincipal const& lb);
    void writeRun(RunPrincipal const& r);
    void writeFileFormatVersion();
    void writeFileIdentifier();
    void writeFileIndex();
    void writeEventHistory();
    void writeProcessConfigurationRegistry();
    void writeProcessHistoryRegistry();
    void writeParameterSetRegistry();
    void writeProductDescriptionRegistry();
    void writeParentageRegistry();
    void writeBranchIDListRegistry();
    void writeProductDependencies();

    void finishEndFile();
    void beginInputFile(FileBlock const& fb, bool fastClone);
    void respondToCloseInputFile(FileBlock const& fb);
    bool shouldWeCloseFile() const;

  private:

    //-------------------------------
    // Private functions

    void setBranchAliases(TTree *tree, Selections const& branches) const;

    void fillBranches(BranchType const& branchType,
                      Principal const& principal,
                      std::vector<ProductProvenance> * productProvenanceVecPtr);

     void insertAncestors(ProductProvenance const& iGetParents,
                          Principal const& principal,
                          std::set<ProductProvenance>& oToFill);
     void fillPsetMap(ParameterSetMap &psetMap);

    //-------------------------------
    // Member data

    std::string file_;
    std::string logicalFile_;
    RootOutput const* om_;
    bool currentlyFastCloning_;
    boost::shared_ptr<TFile> filePtr_;
    FileID fid_;
    FileIndex fileIndex_;
    FileIndex::EntryNumber_t eventEntryNumber_;
    FileIndex::EntryNumber_t subRunEntryNumber_;
    FileIndex::EntryNumber_t runEntryNumber_;
    TTree * metaDataTree_;
    TTree * parentageTree_;
    TTree * eventHistoryTree_;
    EventAuxiliary const*           pEventAux_;
    SubRunAuxiliary const* pSubRunAux_;
    RunAuxiliary const*             pRunAux_;
    ProductProvenanceVector         eventEntryInfoVector_;
    ProductProvenanceVector         subRunEntryInfoVector_;
    ProductProvenanceVector         runEntryInfoVector_;
    ProductProvenanceVector *       pEventEntryInfoVector_;
    ProductProvenanceVector *       pSubRunEntryInfoVector_;
    ProductProvenanceVector *       pRunEntryInfoVector_;
    History const*                  pHistory_;
    RootOutputTree eventTree_;
    RootOutputTree subRunTree_;
    RootOutputTree runTree_;
    RootOutputTreePtrArray treePointers_;
    bool dataTypeReported_;
    std::set<BranchID> branchesWithStoredHistory_;
  };  // RootOutputFile

}  // art

// ======================================================================

#endif
