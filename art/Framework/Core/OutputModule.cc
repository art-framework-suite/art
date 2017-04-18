#include "art/Framework/Core/OutputModule.h"

#include "art/Framework/Core/CPCSentry.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/detail/OutputModuleUtils.h"
#include "art/Framework/Principal/CurrentProcessingContext.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/ParentageRegistry.h"
#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/canonical_string.h"
#include "cetlib_except/demangle.h"
#include "cetlib/exempt_ptr.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include <utility>

using fhicl::ParameterSet;
using std::vector;
using std::string;

art::OutputModule::
OutputModule(fhicl::TableFragment<Config> const& config,
             ParameterSet const& containing_pset)
  : EventObserverBase{config().eoFragment().selectEvents(), containing_pset}
  , groupSelectorRules_{config().outputCommands(), "outputCommands", "OutputModule"}
  , configuredFileName_{config().fileName()}
  , dataTier_{config().dataTier()}
  , streamName_{config().streamName()}
  , plugins_{makePlugins_(containing_pset)}
{}

art::OutputModule::
OutputModule(ParameterSet const& pset)
  : EventObserverBase{pset}
  , groupSelectorRules_{pset.get<vector<string>>("outputCommands", {"keep *"}),
        "outputCommands",
        "OutputModule"}
  , configuredFileName_{pset.get<string>("fileName","")}
  , dataTier_{pset.get<string>("dataTier","")}
  , streamName_{pset.get<string>("streamName","")}
  , plugins_{makePlugins_(pset)}
{}

string const &
art::OutputModule::
lastClosedFileName() const
{
  return configuredFileName_;
}

void
art::OutputModule::
configure(OutputModuleDescription const& desc)
{
  remainingEvents_ = maxEvents_ = desc.maxEvents_;
}

void
art::OutputModule::
doSelectProducts()
{
  auto const& pmd = ProductMetaData::instance();
  groupSelector_.initialize(groupSelectorRules_,pmd.productList());
  for (auto& val : keptProducts_) {
    val.clear();
  }
  // TODO: See if we can collapse keptProducts_ and groupSelector_ into a
  // single object. See the notes in the header for GroupSelector
  // for more information.

  for (auto const& val : pmd.productList()) {
    BranchDescription const& bd = val.second;
    auto const bid = bd.branchID();
    auto const bt = bd.branchType();
    if (bd.transient()) {
      // Transient, skip it.
      continue;
    }
    if (!pmd.produced(bt, bid) &&
        pmd.presentWithFileIdx(bt, bid) == MasterProductRegistry::DROPPED) {
      // Not produced in this process, and previously dropped, skip it.
      continue;
    }
    if (groupSelector_.selected(bd)) {
      // Selected, keep it.
      keptProducts_[bt].push_back(&bd);
      continue;
    }
    // Newly dropped, skip it.
    hasNewlyDroppedBranch_[bt] = true;
  }
}

void
art::OutputModule::
selectProducts(FileBlock const& fb)
{
  preSelectProducts(fb);
  doSelectProducts();
  postSelectProducts(fb);
}

void
art::OutputModule::
registerProducts(MasterProductRegistry& mpr,
                 ModuleDescription const& md)
{
  doRegisterProducts(mpr, md);
}

void
art::OutputModule::
preSelectProducts(FileBlock const&)
{
}

void
art::OutputModule::
postSelectProducts(FileBlock const&)
{
}

void
art::OutputModule::
doRegisterProducts(MasterProductRegistry&,
                   ModuleDescription const&)
{
}

void
art::OutputModule::
doBeginJob()
{
  doSelectProducts();
  beginJob();
  cet::for_all(plugins_, [](auto& p){ p->doBeginJob(); });
}

bool
art::OutputModule::
doBeginRun(RunPrincipal const & rp,
           CurrentProcessingContext const * cpc)
{
  detail::CPCSentry sentry{current_context_, cpc};
  FDEBUG(2) << "beginRun called\n";
  beginRun(rp);
  Run const r {rp, moduleDescription_};
  cet::for_all(plugins_, [&r](auto& p){ p->doBeginRun(r); });
  return true;
}

bool
art::OutputModule::
doBeginSubRun(SubRunPrincipal const& srp,
              CurrentProcessingContext const* cpc)
{
  detail::CPCSentry sentry {current_context_, cpc};
  FDEBUG(2) << "beginSubRun called\n";
  beginSubRun(srp);
  SubRun const sr {srp, moduleDescription_};
  cet::for_all(plugins_, [&sr](auto& p){ p->doBeginSubRun(sr); });
  return true;
}

bool
art::OutputModule::
doEvent(EventPrincipal const& ep, CurrentProcessingContext const* cpc, CountingStatistics& counts)
{
  detail::CPCSentry sentry {current_context_, cpc};
  FDEBUG(2) << "doEvent called\n";
  Event const e {ep, moduleDescription_};
  if (wantAllEvents() || wantEvent(e)) {
    // Run is incremented before analyze(e); to properly count
    // whenever an exception is thrown in the user's module.
    counts.increment<stats::Run>();
    event(ep);
    counts.increment<stats::Passed>();
  }
  return true;
}

void
art::OutputModule::
doWriteEvent(EventPrincipal& ep)
{
  detail::PVSentry clearTriggerResults {cachedProducts()};
  FDEBUG(2) << "writeEvent called\n";
  Event const e {ep, moduleDescription_};
  if (wantAllEvents() || wantEvent(e)) {
    write(ep); // Write the event.
    // Declare that the event was selected for write to the catalog
    // interface
    art::Handle<art::TriggerResults> trHandle {getTriggerResults(e)};
    auto const & trRef ( trHandle.isValid() ? static_cast<HLTGlobalStatus>(*trHandle) : HLTGlobalStatus{} );
    ci_->eventSelected(moduleDescription_.moduleLabel(), ep.id(), trRef);
    // ... and invoke the plugins:
    cet::for_all(plugins_, [&e](auto& p){ p->doCollectMetadata(e); });
    // Finish.
    updateBranchParents(ep);
    if (remainingEvents_ > 0) {
      --remainingEvents_;
    }
  }
}

void
art::OutputModule::
doSetSubRunAuxiliaryRangeSetID(RangeSet const& ranges)
{
  setSubRunAuxiliaryRangeSetID(ranges);
}

bool
art::OutputModule::
doEndSubRun(SubRunPrincipal const& srp,
            CurrentProcessingContext const* cpc)
{
  detail::CPCSentry sentry{current_context_, cpc};
  FDEBUG(2) << "endSubRun called\n";
  endSubRun(srp);
  SubRun const sr {srp, moduleDescription_};
  cet::for_all(plugins_, [&sr](auto& p){ p->doEndSubRun(sr); });
  return true;
}

void
art::OutputModule::
doWriteSubRun(SubRunPrincipal& srp)
{
  FDEBUG(2) << "writeSubRun called\n";
  writeSubRun(srp);
}

void
art::OutputModule::
doSetRunAuxiliaryRangeSetID(RangeSet const& ranges)
{
  FDEBUG(2) << "writeAuxiliaryRangeSets(rp) called\n";
  setRunAuxiliaryRangeSetID(ranges);
}

bool
art::OutputModule::
doEndRun(RunPrincipal const & rp,
         CurrentProcessingContext const * cpc)
{
  detail::CPCSentry sentry {current_context_, cpc};
  FDEBUG(2) << "endRun called\n";
  endRun(rp);
  Run const r {rp, moduleDescription_};
  cet::for_all(plugins_, [&r](auto& p){ p->doEndRun(r); });
  return true;
}

void
art::OutputModule::
doWriteRun(RunPrincipal & rp)
{
  FDEBUG(2) << "writeRun called\n";
  writeRun(rp);
}

void
art::OutputModule::
doEndJob()
{
  endJob();
  cet::for_all(plugins_, [](auto& p){ p->doEndJob(); });
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
  std::unique_ptr<ResultsPrincipal> respHolder;
  art::ResultsPrincipal const * respPtr = fb.resultsPrincipal();
  if (respPtr == nullptr) {
    respHolder = std::make_unique<ResultsPrincipal>(ResultsAuxiliary { },
                                                    description().processConfiguration());
    respPtr = respHolder.get();
  }
  readResults(*respPtr);
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

void
art::OutputModule::
updateBranchParents(EventPrincipal const & ep)
{
  for (auto const& groupPr : ep) {
    auto const& group = *groupPr.second;
    if (group.productProvenancePtr()) {
      BranchID const bid = groupPr.first;
      auto it = branchParents_.find(bid);
      if (it == branchParents_.end()) {
        it = branchParents_.emplace(bid, std::set<ParentageID>()).first;
      }
      it->second.insert(group.productProvenancePtr()->parentageID());
      branchChildren_.insertEmpty(bid);
    }
  }
}

void
art::OutputModule::
fillDependencyGraph()
{
  for (auto const& bp : branchParents_) {
    BranchID const child = bp.first;
    std::set<ParentageID> const & eIds = bp.second;
    for (auto const& eId : eIds) {
      Parentage par;
      if (!ParentageRegistry::get(eId, par)) {
        continue;
      }
      for (auto const& p : par.parents())
        branchChildren_.insertChild(p, child);
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
event(EventPrincipal const&)
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
setRunAuxiliaryRangeSetID(RangeSet const&)
{
}

void
art::OutputModule::
setSubRunAuxiliaryRangeSetID(RangeSet const&)
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
readResults(ResultsPrincipal const &)
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
art::OutputModule::setFileStatus(OutputFileStatus const)
{
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

namespace {
  void
  collectStreamSpecificMetadata(vector<std::unique_ptr<art::FileCatalogMetadataPlugin>> const & plugins,
                                vector<string> const & pluginNames,
                                art::FileCatalogMetadata::collection_type & ssmd)
  {
    std::size_t pluginCounter {0};
    std::ostringstream errors;  // Collect errors from all plugins.
    for (auto & plugin : plugins) {
      art::FileCatalogMetadata::collection_type tmp = plugin->doProduceMetadata();
      ssmd.reserve(tmp.size() + ssmd.size());
      for (auto&& entry : tmp) {
        if (art::ServiceHandle<art::FileCatalogMetadata const>{}->wantCheckSyntax()) {
          rapidjson::Document d;
          string checkString("{ ");
          checkString += cet::canonical_string(entry.first) +
                         " : " +
                         entry.second +
                         " }";
          if (d.Parse(checkString.c_str()).HasParseError()) {
            auto const nSpaces = d.GetErrorOffset();
            std::cerr << "nSpaces = " << nSpaces << ".\n";
            errors
              << "art::OutputModule::writeCatalogMetadata():"
              << "syntax error in metadata produced by plugin "
              << pluginNames[pluginCounter]
              << ":\n"
              << rapidjson::GetParseError_En(d.GetParseError())
              << " Faulty key/value clause:\n"
              << checkString << "\n"
              << (nSpaces ? string(nSpaces, '-') : "")
              << "^\n";
          }
        }
        ssmd.emplace_back(std::move(entry));
      }
      ++pluginCounter;
    }
    auto const errMsg = errors.str();
    if (!errMsg.empty()) {
      throw art::Exception(art::errors::DataCorruption) << errMsg;
    }
  }
}

void
art::OutputModule::
writeFileCatalogMetadata()
{
  // Obtain metadata from service for output.
  FileCatalogMetadata::collection_type md, ssmd;
  ServiceHandle<FileCatalogMetadata const>{}->getMetadata(md);
  if (!dataTier_.empty()) {
    md.emplace_back("data_tier", cet::canonical_string(dataTier_));
  }
  if (!streamName_.empty()) {
    md.emplace_back("data_stream", cet::canonical_string(streamName_));
  }
  // Ask any plugins for their list of metadata, and put it in a
  // separate list for the output module. The user stream-specific
  // metadata should override stream-specific metadata generated by the
  // output module iself.
  collectStreamSpecificMetadata(plugins_, pluginNames_, ssmd);
  doWriteFileCatalogMetadata(md, ssmd);
}

void
art::OutputModule::
doWriteFileCatalogMetadata(FileCatalogMetadata::collection_type const &,
                           FileCatalogMetadata::collection_type const &)
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

auto
art::OutputModule::
makePlugins_(ParameterSet const & top_pset)
  -> PluginCollection_t
{
  auto const psets = top_pset.get<vector<ParameterSet>>("FCMDPlugins", {} );
  PluginCollection_t result;
  result.reserve(psets.size());
  size_t count {0};
  try {
    for (auto const & pset : psets) {
      pluginNames_.emplace_back(pset.get<string>("plugin_type"));
      auto const & libspec = pluginNames_.back();
      auto const pluginType = pluginFactory_.pluginType(libspec);
      if (pluginType == cet::PluginTypeDeducer<FileCatalogMetadataPlugin>::value) {
        result.emplace_back(pluginFactory_.
                            makePlugin<std::unique_ptr<FileCatalogMetadataPlugin> >(libspec, pset));
      } else {
        throw Exception(errors::Configuration, "OutputModule: ")
          << "unrecognized plugin type "
          << pluginType
          << ".\n";
      }
      ++count;
    }
  }
  catch (cet::exception & e) {
    throw Exception(errors::Configuration, "OutputModule: ", e)
      << "Exception caught while processing FCMDPlugins["
      << count
      << "] in module "
      << description().moduleLabel()
      << ".\n";
  }
  return result;
}
