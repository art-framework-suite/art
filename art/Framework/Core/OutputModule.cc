#include "art/Framework/Core/OutputModule.h"

#include "art/Framework/Core/CPCSentry.h"
#include "art/Framework/Core/detail/OutputModuleUtils.h"
#include "art/Framework/Principal/CurrentProcessingContext.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/ParentageRegistry.h"
#include "art/Utilities/DebugMacros.h"
#include "art/Utilities/Exception.h"
#include "cetlib/demangle.h"
#include "cpp0x/utility"


using fhicl::ParameterSet;
using std::vector;
using std::string;


namespace art {
  vector<std::string> const & getAllTriggerNames()
  {
    art::ServiceHandle<art::TriggerNamesService> tns;
    return tns->getTrigPaths();
  }
}

art::OutputModule::
OutputModule(ParameterSet const & pset)
  :
  maxEvents_(-1),
  remainingEvents_(maxEvents_),
  keptProducts_(),
  hasNewlyDroppedBranch_(),
  process_name_(),
  groupSelectorRules_(pset, "outputCommands", "OutputModule"),
  groupSelector_(),
  moduleDescription_(),
  current_context_(0),
  prodsValid_(false),
  wantAllEvents_(false),
  selectors_(),
  selector_config_id_(),
  branchParents_(),
  branchChildren_(),
  configuredFileName_(pset.get<std::string>("fileName", "")),
  dataTier_(pset.get<std::string>("dataTier", "")),
  streamName_(pset.get<std::string>("streamName", "")),
  ci_()
{
  hasNewlyDroppedBranch_.fill(false);
  ServiceHandle<art::TriggerNamesService> tns;
  process_name_ = tns->getProcessName();
  ParameterSet selectevents =
    pset.get<fhicl::ParameterSet>("SelectEvents", ParameterSet());
  selector_config_id_ = selectevents.id();
  // If selectevents is an emtpy ParameterSet, then we are to write
  // all events, or one which contains a vstrig 'SelectEvents' that
  // is empty, we are to write all events. We have no need for any
  // EventSelectors.
  if (selectevents.is_empty()) {
    wantAllEvents_ = true;
    selectors_.setupDefault(getAllTriggerNames());
    return;
  }
  vector<std::string> path_specs =
    selectevents.get<std::vector<std::string> >("SelectEvents");
  if (path_specs.empty()) {
    wantAllEvents_ = true;
    selectors_.setupDefault(getAllTriggerNames());
    return;
  }
  // If we get here, we have the possibility of having to deal with
  // path_specs that look at more than one process.
  vector<detail::parsed_path_spec_t> parsed_paths(path_specs.size());
  for (size_t i = 0; i < path_specs.size(); ++i)
  { detail::parse_path_spec(path_specs[i], parsed_paths[i]); }
  selectors_.setup(parsed_paths, getAllTriggerNames(), process_name_);
}

std::string const &
art::OutputModule::
lastClosedFileName() const
{
  return configuredFileName_;
}

void
art::OutputModule::
configure(OutputModuleDescription const & desc)
{
  remainingEvents_ = maxEvents_ = desc.maxEvents_;
}

void
art::OutputModule::
selectProducts()
{
  if (groupSelector_.initialized()) { return; }
  ProductList const & pList = ProductMetaData::instance().productList();
  groupSelector_.initialize(groupSelectorRules_, pList);
  // TODO: See if we can collapse keptProducts_ and groupSelector_ into a
  // single object. See the notes in the header for GroupSelector
  // for more information.
  for (ProductList::const_iterator
       it = pList.begin(),
       end = pList.end();
       it != end;
       ++it) {
    BranchDescription const & desc = it->second;
    if (desc.transient()) {
      // if the class of the branch is marked transient, output nothing
    }
    else if (!desc.present() && !desc.produced()) {
      // else if the branch containing the product has been previously dropped,
      // output nothing
    }
    else if (selected(desc)) {
      // else if the branch has been selected, put it in the list of selected branches
      keptProducts_[desc.branchType()].push_back(&desc);
    }
    else {
      // otherwise, output nothing,
      // and mark the fact that there is a newly dropped branch of this type.
      hasNewlyDroppedBranch_[desc.branchType()] = true;
    }
  }
}

void
art::OutputModule::
reconfigure(ParameterSet const &)
{
  throw art::Exception(errors::UnimplementedFeature)
    << "Modules of type "
    << cet::demangle_symbol(typeid(*this).name())
    << " are not reconfigurable.\n";
}

void
art::OutputModule::
doBeginJob()
{
  selectProducts();
  beginJob();
}

void
art::OutputModule::
doEndJob()
{
  endJob();
}


art::Trig
art::OutputModule::getTriggerResults(Event const & e) const
{
  return selectors_.getOneTriggerResults(e);
}

namespace {
  class PVSentry {
  public:
    PVSentry(art::detail::CachedProducts & prods, bool & valid) : p(prods),
      v(valid) {}
    ~PVSentry() { p.clear(); v = false; }
  private:
    art::detail::CachedProducts & p;
    bool & v;

    PVSentry(PVSentry const &) = delete;
    PVSentry & operator=(PVSentry const &) = delete;
  };
}

bool
art::OutputModule::
doEvent(EventPrincipal const & ep,
        CurrentProcessingContext const * cpc)
{
  detail::CPCSentry sentry(current_context_, cpc);
  PVSentry products_sentry(selectors_, prodsValid_);
  FDEBUG(2) << "writeEvent called\n";
  Event e(const_cast<EventPrincipal &>(ep), moduleDescription_);
  if (wantAllEvents_ || selectors_.wantEvent(e)) {
    write(ep); // Write the event.
    // Declare that the event was selected for write.
    Trig trHandle(getTriggerResults(e));
    HLTGlobalStatus const &
    trRef(trHandle.isValid() ?
          static_cast<HLTGlobalStatus>(*trHandle) :
          HLTGlobalStatus());
    ci_->eventSelected(moduleDescription_.moduleLabel(),
                       ep.id(),
                       trRef);
    // Finish.
    updateBranchParents(ep);
    if (remainingEvents_ > 0) {
      --remainingEvents_;
    }
  }
  return true;
}

bool
art::OutputModule::
doBeginRun(RunPrincipal const & rp,
           CurrentProcessingContext const * cpc)
{
  detail::CPCSentry sentry(current_context_, cpc);
  FDEBUG(2) << "beginRun called\n";
  beginRun(rp);
  return true;
}

bool
art::OutputModule::
doEndRun(RunPrincipal const & rp,
         CurrentProcessingContext const * cpc)
{
  detail::CPCSentry sentry(current_context_, cpc);
  FDEBUG(2) << "endRun called\n";
  endRun(rp);
  return true;
}

void
art::OutputModule::
doWriteRun(RunPrincipal const & rp)
{
  FDEBUG(2) << "writeRun called\n";
  writeRun(rp);
}

bool
art::OutputModule::
doBeginSubRun(SubRunPrincipal const & srp,
              CurrentProcessingContext const * cpc)
{
  detail::CPCSentry sentry(current_context_, cpc);
  FDEBUG(2) << "beginSubRun called\n";
  beginSubRun(srp);
  return true;
}

bool
art::OutputModule::
doEndSubRun(SubRunPrincipal const & srp,
            CurrentProcessingContext const * cpc)
{
  detail::CPCSentry sentry(current_context_, cpc);
  FDEBUG(2) << "endSubRun called\n";
  endSubRun(srp);
  return true;
}

void
art::OutputModule::
doWriteSubRun(SubRunPrincipal const & srp)
{
  FDEBUG(2) << "writeSubRun called\n";
  writeSubRun(srp);
}

void
art::OutputModule::doOpenFile(FileBlock const & fb)
{
  openFile(fb);
}

void
art::OutputModule::
doRespondToOpenInputFile(FileBlock const & fb)
{
  respondToOpenInputFile(fb);
}

void
art::OutputModule::
doRespondToCloseInputFile(FileBlock const & fb)
{
  respondToCloseInputFile(fb);
}

void
art::OutputModule::
doRespondToOpenOutputFiles(FileBlock const & fb)
{
  respondToOpenOutputFiles(fb);
}

void
art::OutputModule::
doRespondToCloseOutputFiles(FileBlock const & fb)
{
  respondToCloseOutputFiles(fb);
}

void
art::OutputModule::
doCloseFile()
{
  if (isFileOpen()) { reallyCloseFile(); }
}

void
art::OutputModule::
reallyCloseFile()
{
  fillDependencyGraph();
  startEndFile();
  writeFileFormatVersion();
  writeFileIdentifier();
  writeFileIndex();
  writeEventHistory();
  writeProcessConfigurationRegistry();
  writeProcessHistoryRegistry();
  writeParameterSetRegistry();
  writeProductDescriptionRegistry();
  writeParentageRegistry();
  writeBranchIDListRegistry();
  writeFileCatalogMetadata();
  writeProductDependencies();
  writeBranchMapper();
  finishEndFile();
  branchParents_.clear();
  branchChildren_.clear();
}

art::CurrentProcessingContext const *
art::OutputModule::
currentContext() const
{
  return current_context_;
}

art::ModuleDescription const &
art::OutputModule::
description() const
{
  return moduleDescription_;
}

bool
art::OutputModule::
selected(BranchDescription const & desc) const
{
  return groupSelector_.selected(desc);
}

void
art::OutputModule::
updateBranchParents(EventPrincipal const & ep)
{
  for (EventPrincipal::const_iterator i = ep.begin(), iEnd = ep.end(); i != iEnd;
       ++i) {
    if (i->second->productProvenancePtr()) {
      BranchID const & bid = i->first;
      BranchParents::iterator it = branchParents_.find(bid);
      if (it == branchParents_.end()) {
        it = branchParents_.insert(std::make_pair(bid, std::set<ParentageID>())).first;
      }
      it->second.insert(i->second->productProvenancePtr()->parentageID());
      branchChildren_.insertEmpty(bid);
    }
  }
}

void
art::OutputModule::
fillDependencyGraph()
{
  for (BranchParents::const_iterator i = branchParents_.begin(),
       iEnd = branchParents_.end();
       i != iEnd; ++i) {
    BranchID const & child = i->first;
    std::set<ParentageID> const & eIds = i->second;
    for (std::set<ParentageID>::const_iterator it = eIds.begin(),
         itEnd = eIds.end();
         it != itEnd; ++it) {
      Parentage entryDesc;
      ParentageRegistry::get(*it, entryDesc);
      std::vector<BranchID> const & parents = entryDesc.parents();
      for (std::vector<BranchID>::const_iterator j = parents.begin(),
           jEnd = parents.end();
           j != jEnd; ++j) {
        branchChildren_.insertChild(*j, child);
      }
    }
  }
}

void
art::OutputModule::
beginJob()
{
}

void
art::OutputModule::
endJob()
{
}

void
art::OutputModule::
beginRun(RunPrincipal const &)
{
}

void
art::OutputModule::
endRun(RunPrincipal const &)
{
}

void
art::OutputModule::
beginSubRun(SubRunPrincipal const &)
{
}

void
art::OutputModule::
endSubRun(SubRunPrincipal const &)
{
}

void
art::OutputModule::
openFile(FileBlock const &)
{
}

void
art::OutputModule::
respondToOpenInputFile(FileBlock const &)
{
}

void
art::OutputModule::
respondToCloseInputFile(FileBlock const &)
{
}

void
art::OutputModule::
respondToOpenOutputFiles(FileBlock const &)
{
}

void
art::OutputModule::
respondToCloseOutputFiles(FileBlock const &)
{
}

bool
art::OutputModule::
isFileOpen() const
{
  return true;
}

void
art::OutputModule::
startEndFile()
{
}

void
art::OutputModule::
writeFileFormatVersion()
{
}

void
art::OutputModule::
writeFileIdentifier()
{
}

void
art::OutputModule::
writeFileIndex()
{
}

void
art::OutputModule::
writeEventHistory()
{
}

void
art::OutputModule::
writeProcessConfigurationRegistry()
{
}

void
art::OutputModule::
writeProcessHistoryRegistry()
{
}

void
art::OutputModule::
writeParameterSetRegistry()
{
}

void
art::OutputModule::
writeBranchIDListRegistry()
{
}

void
art::OutputModule::
writeParentageRegistry()
{
}

void
art::OutputModule::
writeProductDescriptionRegistry()
{
}

void
art::OutputModule::
writeFileCatalogMetadata()
{
  if (!dataTier_.empty()) {
    // Add output module-specific info and tell our concrete class to write it out.
    FileCatalogMetadata::collection_type md;
    ServiceHandle<FileCatalogMetadata>()->getMetadata(md);
    md.emplace_back("dataTier", dataTier_);
    md.emplace_back("streamName", streamName_);
    doWriteFileCatalogMetadata(md);
  }
}

void
art::OutputModule::
doWriteFileCatalogMetadata(FileCatalogMetadata::collection_type const &)
{
}

void
art::OutputModule::
writeProductDependencies()
{
}

void
art::OutputModule::
writeBranchMapper()
{
}

void
art::OutputModule::
finishEndFile()
{
}
