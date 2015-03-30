// ======================================================================
//
// RootOutput
//
// ======================================================================

#include "art/Framework/IO/Root/RootOutput.h"

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/IO/PostCloseFileRenamer.h"
#include "art/Framework/IO/Root/RootOutputFile.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/unique_filename.h"
#include "art/Utilities/parent_path.h"
#include "cetlib/container_algorithms.h"
#include "cpp0x/utility"
#include "fhiclcpp/ParameterSet.h"

#include "TBranchElement.h"
#include "TObjArray.h"
#include "TTree.h"

#include <iomanip>
#include <sstream>

using art::RootOutput;
using fhicl::ParameterSet;
using std::string;

namespace {
  bool fastCloningDefault = true;

  bool setAndReportFastCloning(fhicl::ParameterSet const & ps)
  {
    bool result = fastCloningDefault;
    mf::LogInfo msg("FastCloning");
    msg << "Initial fast cloning configuration ";
    if (ps.get_if_present("fastCloning", result)) {
      msg << "(user-set):";
    } else {
      msg << "(from default):";
    }
    msg << std::boolalpha << result;
    return result;
  }
}

namespace art {

  RootOutput::RootOutput(ParameterSet const& ps)
  : OutputModule               ( ps )
  , selectedOutputItemList_    ( )
  , catalog_                   ( ps.get<string>("catalog", string()) )
  , maxFileSize_               ( ps.get<int>("maxSize", 0x7f000000) )
  , compressionLevel_          ( ps.get<int>("compressionLevel", 7) )
  , basketSize_                ( ps.get<int>("basketSize", 16384) )
  , splitLevel_                ( ps.get<int>("splitLevel", 99) )
  , treeMaxVirtualSize_        ( ps.get<int64_t>("treeMaxVirtualSize", -1) )
  , saveMemoryObjectThreshold_ ( ps.get<int64_t>("saveMemoryObjectThreshold", -1l) )
  , fastCloning_               ( setAndReportFastCloning(ps) )
  , dropAllEvents_             ( false )
  , dropAllSubRuns_            ( ps.get<bool>("dropAllSubRuns", false) )
  , dropMetaData_              ( DropNone )  // tentative: see the c'tor body
  , dropMetaDataForDroppedData_( ps.get<bool>( "dropMetaDataForDroppedData"
                                             , false) )
  , moduleLabel_               ( ps.get<string>("module_label")  )
  , inputFileCount_            ( 0 )
  , rootOutputFile_            ( )
  , fstats_                    ( moduleLabel_, processName() )
  , filePattern_               ( ps.get<string>("fileName") )
  , tmpDir_                    ( ps.get<string>("tmpDir",
                                                parent_path(filePattern_)) )
  , lastClosedFileName_        ( )
  {
    if (fastCloning_ && !wantAllEvents()) {
      fastCloning_ = false;
      mf::LogWarning("FastCloning")
        << "Fast cloning deactivated due to presence of event selection configuration.";
    }

    bool const dropAllEventsSet = ps.get_if_present<bool>("dropAllEvents", dropAllEvents_ );

    if ( dropAllSubRuns_ ) {
      if ( dropAllEventsSet && !dropAllEvents_ ) {
        std::string const errmsg =
          "\nThe following FHiCL specification is illegal\n\n"
          "   dropAllEvents  : false \n"
          "   dropAllSubRuns : true  \n\n"
          "[1] Both can be 'true', "
          "[2] both can be 'false', or "
          "[3] 'dropAllEvents : true' and 'dropAllSubRuns : false' is allowed.\n\n";
        throw art::Exception( errors::Configuration, errmsg );
      }
      dropAllEvents_ = true;
    }

    string dropMetaData(ps.get<string>("dropMetaData", string()));
    if (dropMetaData.empty())                 dropMetaData_ = DropNone;
    else if (dropMetaData == string("NONE"))  dropMetaData_ = DropNone;
    else if (dropMetaData == string("PRIOR")) dropMetaData_ = DropPrior;
    else if (dropMetaData == string("ALL"))   dropMetaData_ = DropAll;
    else {
      throw art::Exception( errors::Configuration
                            , "Illegal dropMetaData parameter value: ")
        << dropMetaData << ".\n"
        << "Legal values are 'NONE', 'PRIOR', and 'ALL'.\n";
    }
  }  // c'tor

  RootOutput::OutputItem::Sorter::Sorter(TTree * tree) {
    // Fill a map mapping branch names to an index specifying the order in the tree.
    if (tree != 0) {
      TObjArray * branches = tree->GetListOfBranches();
      for (int i = 0, sz = branches->GetEntries(); i != sz; ++i) {
        TBranchElement * br = (TBranchElement *)branches->At(i);
        treeMap_.insert(std::make_pair(string(br->GetName()), i));
      }
    }
  }

  bool
  RootOutput::OutputItem::Sorter::operator()(OutputItem const& lh, OutputItem const& rh) const {
    // Provides a comparison for sorting branches according to the index values in treeMap_.
    // Branches not found are always put at the end (i.e. not found > found).
    if (treeMap_.empty()) return lh < rh;
    string const& lstring = lh.branchDescription_->branchName();
    string const& rstring = rh.branchDescription_->branchName();
    std::map<string, int>::const_iterator lit = treeMap_.find(lstring);
    std::map<string, int>::const_iterator rit = treeMap_.find(rstring);
    bool lfound = (lit != treeMap_.end());
    bool rfound = (rit != treeMap_.end());
    if (lfound && rfound) {
      return lit->second < rit->second;
    } else if (lfound) {
      return true;
    } else if (rfound) {
      return false;
    }
    return lh < rh;
  }

  void RootOutput::fillSelectedItemList(BranchType branchType, TTree * theTree) {
    Selections const& keptVector =    keptProducts()[branchType];
    OutputItemList&   outputItemList = selectedOutputItemList_[branchType];
    for (auto const& selection : keptVector) outputItemList.push_back(OutputItem(selection));
    // Sort outputItemList to allow fast copying.  The branches in
    // outputItemList must be in the same order as in the input tree,
    // with all new branches at the end.
    cet::sort_all(outputItemList, OutputItem::Sorter(theTree));
  }

  void RootOutput::openFile(FileBlock const& fb) {
    if (!isFileOpen()) {
      doOpenFile();
      respondToOpenInputFile(fb);
    }
  }

  void RootOutput::respondToOpenInputFile(FileBlock const& fb) {
    for (int i = InEvent; i < NumBranchTypes; ++i) {
      BranchType branchType = static_cast<BranchType>(i);
      if (inputFileCount_ == 0) {
        TTree * theTree = (branchType == InEvent ? fb.tree() :
                          (branchType == InSubRun ? fb.subRunTree() :
                          fb.runTree()));
        fillSelectedItemList(branchType, theTree);
      }
    }
    ++inputFileCount_;
    if (isFileOpen()) {
      bool fastCloneThisOne = fastCloning_ &&
                              fb.tree() != 0 &&
                              (remainingEvents() < 0 || remainingEvents() >= fb.tree()->GetEntries());
      if (fastCloning_ && !fastCloneThisOne) {
        mf::LogWarning("FastCloning")
          << "Fast cloning deactivated for this input file due to "
          << "empty event tree and/or event limits.";
      }
      if (fastCloneThisOne && ! fb.fastClonable()) {
        mf::LogWarning("FastCloning")
          << "Fast cloning deactivated for this input file due to "
          << "information in FileBlock.";
        fastCloneThisOne = false;
      }
      rootOutputFile_->beginInputFile(fb, fastCloneThisOne && fastCloning_);
      fstats_.recordInputFile(fb.fileName());
    }
  }

  void RootOutput::respondToCloseInputFile(FileBlock const& fb) {
    if (rootOutputFile_) rootOutputFile_->respondToCloseInputFile(fb);
  }

  RootOutput::~RootOutput() {
  }

  void RootOutput::write(EventPrincipal const& e)
  {
    if (dropAllEvents_) return;
    if (hasNewlyDroppedBranch()[InEvent]) e.addToProcessHistory();
    rootOutputFile_->writeOne(e);
    fstats_.recordEvent(e.id());
  }

  void RootOutput::writeSubRun(SubRunPrincipal const& sr)
  {
    if (dropAllSubRuns_) return;
    if (hasNewlyDroppedBranch()[InSubRun]) sr.addToProcessHistory();
    rootOutputFile_->writeSubRun(sr);
    fstats_.recordSubRun(sr.id());
  }

  void RootOutput::writeRun(RunPrincipal const& r)
  {
    if (hasNewlyDroppedBranch()[InRun]) r.addToProcessHistory();
    rootOutputFile_->writeRun(r);
    fstats_.recordRun(r.id());
  }

  // At some later date, we may move functionality from finishEndFile() to here.
  void RootOutput::startEndFile() { }


  void RootOutput::writeFileFormatVersion() { rootOutputFile_->writeFileFormatVersion(); }
  void RootOutput::writeFileIndex() { rootOutputFile_->writeFileIndex(); }
  void RootOutput::writeEventHistory() { rootOutputFile_->writeEventHistory(); }
  void RootOutput::writeProcessConfigurationRegistry() { rootOutputFile_->writeProcessConfigurationRegistry(); }
  void RootOutput::writeProcessHistoryRegistry() { rootOutputFile_->writeProcessHistoryRegistry(); }
  void RootOutput::writeParameterSetRegistry() { rootOutputFile_->writeParameterSetRegistry(); }
  void RootOutput::writeProductDescriptionRegistry() { rootOutputFile_->writeProductDescriptionRegistry(); }
  void RootOutput::writeParentageRegistry() { rootOutputFile_->writeParentageRegistry(); }
  void RootOutput::writeBranchIDListRegistry() { rootOutputFile_->writeBranchIDListRegistry(); }

  void
  RootOutput::
  doWriteFileCatalogMetadata(FileCatalogMetadata::collection_type const & md,
                             FileCatalogMetadata::collection_type const & ssmd)
  {
    rootOutputFile_->writeFileCatalogMetadata(fstats_, md, ssmd);
  }

  void RootOutput::writeProductDependencies() { rootOutputFile_->writeProductDependencies(); }
  void RootOutput::finishEndFile() {
    rootOutputFile_->finishEndFile();
    fstats_.recordFileClose();
    lastClosedFileName_ =
      PostCloseFileRenamer(fstats_).
      maybeRenameFile(rootOutputFile_->currentFileName(),
                      filePattern_);
    rootOutputFile_.reset();
  }
  bool RootOutput::isFileOpen() const { return rootOutputFile_.get() != 0; }
  bool RootOutput::shouldWeCloseFile() const { return rootOutputFile_->shouldWeCloseFile(); }

  void RootOutput::doOpenFile() {
      if (inputFileCount_ == 0) {
        throw art::Exception(art::errors::LogicError)
          << "Attempt to open output file before input file. "
          << "Please report this to the core framework developers.\n";
      }
      rootOutputFile_.reset(new RootOutputFile(this,
                                               unique_filename(tmpDir_ +
                                                               "/RootOutput")));
      fstats_.recordFileOpen();
  }

  std::string const &
  RootOutput::lastClosedFileName() const {
    if (lastClosedFileName_.empty()) {
      throw Exception(errors::LogicError, "RootOutput::currentFileName(): ")
        << "called before meaningful.\n";
    }
    return lastClosedFileName_;
  }

}  // art

// ======================================================================

DEFINE_ART_MODULE(RootOutput)

// ======================================================================
