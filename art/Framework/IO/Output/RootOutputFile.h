#ifndef IOPool_Output_RootOutputFile_h
#define IOPool_Output_RootOutputFile_h

//////////////////////////////////////////////////////////////////////
//
// Class RootOutputFile
//
// Oringinal Author: Luca Lista
// Current Author: Bill Tanenbaum
//
//////////////////////////////////////////////////////////////////////

#include <map>
#include <string>
#include <vector>

#include "boost/array.hpp"
#include "boost/shared_ptr.hpp"

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/FileID.h"
#include "art/Persistency/Provenance/FileIndex.h"
#include "art/Persistency/Provenance/Selections.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Framework/IO/Output/PoolOutputModule.h"
#include "art/Framework/IO/Output/RootOutputTree.h"

class TTree;
class TFile;
#include "TROOT.h"

namespace edm {
  class PoolOutputModule;
  class History;

  class RootOutputFile {
  public:
    typedef PoolOutputModule::OutputItem OutputItem;
    typedef PoolOutputModule::OutputItemList OutputItemList;
    typedef boost::array<RootOutputTree *, NumBranchTypes> RootOutputTreePtrArray;
    explicit RootOutputFile(PoolOutputModule * om, std::string const& fileName,
                            std::string const& logicalFileName);
    ~RootOutputFile() {}
    void writeOne(EventPrincipal const& e);
    //void endFile();
    void writeLuminosityBlock(SubRunPrincipal const& lb);
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
    // Local types
    //

    //-------------------------------
    // Private functions

    void setBranchAliases(TTree *tree, Selections const& branches) const;

    void fillBranches(BranchType const& branchType,
		      Principal const& principal,
		      std::vector<ProductProvenance> * productProvenanceVecPtr);

     void insertAncestors(ProductProvenance const& iGetParents,
                          Principal const& principal,
                          std::set<ProductProvenance>& oToFill);

    //-------------------------------
    // Member data

    std::string file_;
    std::string logicalFile_;
    PoolOutputModule const* om_;
    bool currentlyFastCloning_;
    boost::shared_ptr<TFile> filePtr_;
    FileID fid_;
    FileIndex fileIndex_;
    FileIndex::EntryNumber_t eventEntryNumber_;
    FileIndex::EntryNumber_t lumiEntryNumber_;
    FileIndex::EntryNumber_t runEntryNumber_;
    TTree * metaDataTree_;
    TTree * parentageTree_;
    TTree * eventHistoryTree_;
    EventAuxiliary const*           pEventAux_;
    SubRunAuxiliary const* pLumiAux_;
    RunAuxiliary const*             pRunAux_;
    ProductProvenanceVector         eventEntryInfoVector_;
    ProductProvenanceVector	    lumiEntryInfoVector_;
    ProductProvenanceVector         runEntryInfoVector_;
    ProductProvenanceVector *       pEventEntryInfoVector_;
    ProductProvenanceVector *       pLumiEntryInfoVector_;
    ProductProvenanceVector *       pRunEntryInfoVector_;
    History const*                  pHistory_;
    RootOutputTree eventTree_;
    RootOutputTree lumiTree_;
    RootOutputTree runTree_;
    RootOutputTreePtrArray treePointers_;
    bool dataTypeReported_;
    std::set<BranchID> branchesWithStoredHistory_;
  };

}

#endif
