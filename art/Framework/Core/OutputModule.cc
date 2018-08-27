#include "art/Framework/Core/OutputModule.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/FileCatalogMetadataPlugin.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/GroupSelector.h"
#include "art/Framework/Core/GroupSelectorRules.h"
#include "art/Framework/Core/Observer.h"
#include "art/Framework/Core/OutputModuleDescription.h"
#include "art/Framework/Core/OutputWorker.h"
#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/RangeSetHandler.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/FileServiceInterfaces/CatalogInterface.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/Selections.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/IDNumber.h"
#include "canvas/Persistency/Provenance/ParentageID.h"
#include "canvas/Persistency/Provenance/ParentageRegistry.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/BasicPluginFactory.h"
#include "cetlib/canonical_string.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/SerialTaskQueueChain.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace hep::concurrency;
using namespace std;

using fhicl::ParameterSet;

namespace art {

  OutputModule::~OutputModule() noexcept = default;

  OutputModule::OutputModule(fhicl::TableFragment<Config> const& config,
                             ParameterSet const& containing_pset)
    : Observer{config().eoFragment().selectEvents(), containing_pset}
    , groupSelectorRules_{config().outputCommands(),
                          "outputCommands",
                          "OutputModule"}
    , configuredFileName_{config().fileName()}
    , dataTier_{config().dataTier()}
    , streamName_{config().streamName()}
    , plugins_{makePlugins_(containing_pset)}
  {}

  OutputModule::OutputModule(ParameterSet const& pset)
    : Observer{pset}
    , groupSelectorRules_{pset.get<vector<string>>("outputCommands",
                                                   {"keep *"}),
                          "outputCommands",
                          "OutputModule"}
    , configuredFileName_{pset.get<string>("fileName", "")}
    , dataTier_{pset.get<string>("dataTier", "")}
    , streamName_{pset.get<string>("streamName", "")}
    , plugins_{makePlugins_(pset)}
  {}

  bool
  OutputModule::fileIsOpen() const
  {
    return isFileOpen();
  }

  string
  OutputModule::workerType() const
  {
    return "OutputWorker";
  }

  void
  OutputModule::incrementInputFileNumber()
  {}

  bool
  OutputModule::requestsToCloseFile() const
  {
    return false;
  }

  Granularity
  OutputModule::fileGranularity() const
  {
    return Granularity::Unset;
  }

  string const&
  OutputModule::lastClosedFileName() const
  {
    return configuredFileName_;
  }

  void
  OutputModule::configure(OutputModuleDescription const& desc)
  {
    remainingEvents_ = maxEvents_ = desc.maxEvents_;
  }

  void
  OutputModule::doSelectProducts(ProductTables const& tables)
  {
    // Note: The keptProducts_ data member records all of the
    // BranchDescription objects that may be persisted to disk.  Since
    // we do not reset it, the list never shrinks.  This behavior should
    // be reconsidered for future use cases of art.
    auto selectProductForBranchType = [this, &tables](BranchType const bt) {
      auto const& productList = tables.descriptions(bt);
      groupSelector_[bt] =
        std::make_unique<GroupSelector const>(groupSelectorRules_, productList);
      // TODO: See if we can collapse keptProducts_ and groupSelector into
      // a single object. See the notes in the header for GroupSelector
      // for more information.
      for (auto const& val : productList) {
        BranchDescription const& pd = val.second;
        if (pd.transient() || pd.dropped()) {
          continue;
        }
        if (selected(pd)) {
          // Here, we take care to merge the BranchDescription objects
          // if one was already present in the keptProducts list.
          auto& keptProducts = keptProducts_[bt];
          auto it = keptProducts.find(pd.productID());
          if (it == end(keptProducts)) {
            // New product
            keptProducts.emplace(pd.productID(), pd);
          } else {
            auto& found_pd = it->second;
            assert(combinable(found_pd, pd));
            found_pd.merge(pd);
          }
          continue;
        }
        hasNewlyDroppedBranch_[bt] = true;
      }
    };
    for_each_branch_type(selectProductForBranchType);
  }

  void
  OutputModule::selectProducts(ProductTables const& tables)
  {
    doSelectProducts(tables);
    postSelectProducts();
  }

  void
  OutputModule::postSelectProducts()
  {}

  void
  OutputModule::registerProducts(ProductDescriptions& producedProducts,
                                 ModuleDescription const& md)
  {
    doRegisterProducts(producedProducts, md);
  }

  void
  OutputModule::doRegisterProducts(ProductDescriptions&,
                                   ModuleDescription const&)
  {}

  void
  OutputModule::doBeginJob()
  {
    serialize(SharedResourcesRegistry::kLegacy);
    createQueues();
    beginJob();
    cet::for_all(plugins_, [](auto& p) { p->doBeginJob(); });
  }

  bool
  OutputModule::doBeginRun(RunPrincipal const& rp, ModuleContext const& mc)
  {
    FDEBUG(2) << "beginRun called\n";
    beginRun(rp);
    Run const r{rp, mc};
    cet::for_all(plugins_, [&r](auto& p) { p->doBeginRun(r); });
    return true;
  }

  bool
  OutputModule::doBeginSubRun(SubRunPrincipal const& srp,
                              ModuleContext const& mc)
  {
    FDEBUG(2) << "beginSubRun called\n";
    beginSubRun(srp);
    SubRun const sr{srp, mc};
    cet::for_all(plugins_, [&sr](auto& p) { p->doBeginSubRun(sr); });
    return true;
  }

  bool
  OutputModule::doEvent(EventPrincipal const& ep,
                        ModuleContext const& mc,
                        std::atomic<std::size_t>& counts_run,
                        std::atomic<std::size_t>& counts_passed,
                        std::atomic<std::size_t>& /*counts_failed*/)
  {
    FDEBUG(2) << "doEvent called\n";
    Event const e{ep, mc};
    if (wantAllEvents() || wantEvent(e)) {
      ++counts_run;
      event(ep);
      ++counts_passed;
    }
    return true;
  }

  void
  OutputModule::doWriteEvent(EventPrincipal& ep)
  {
    detail::PVSentry clearTriggerResults{processAndEventSelectors()};
    FDEBUG(2) << "writeEvent called\n";
    ModuleContext const mc{moduleDescription()};
    Event const e{ep, mc};
    if (wantAllEvents() || wantEvent(e)) {
      write(ep);
      // Declare that the event was selected for write to the catalog interface.
      Handle<TriggerResults> trHandle{getTriggerResults(e)};
      auto const& trRef(trHandle.isValid() ?
                          static_cast<HLTGlobalStatus>(*trHandle) :
                          HLTGlobalStatus{});
      ci_->eventSelected(
        moduleDescription().moduleLabel(), ep.eventID(), trRef);
      // ... and invoke the plugins:
      cet::for_all(plugins_, [&e](auto& p) { p->doCollectMetadata(e); });
      updateBranchParents(ep);
      if (remainingEvents_ > 0) {
        --remainingEvents_;
      }
    }
  }

  void
  OutputModule::doSetSubRunAuxiliaryRangeSetID(RangeSet const& ranges)
  {
    setSubRunAuxiliaryRangeSetID(ranges);
  }

  bool
  OutputModule::doEndSubRun(SubRunPrincipal const& srp, ModuleContext const& mc)
  {
    FDEBUG(2) << "endSubRun called\n";
    endSubRun(srp);
    SubRun const sr{srp, mc};
    cet::for_all(plugins_, [&sr](auto& p) { p->doEndSubRun(sr); });
    return true;
  }

  void
  OutputModule::doWriteSubRun(SubRunPrincipal& srp)
  {
    FDEBUG(2) << "writeSubRun called\n";
    writeSubRun(srp);
  }

  void
  OutputModule::doSetRunAuxiliaryRangeSetID(RangeSet const& ranges)
  {
    FDEBUG(2) << "writeAuxiliaryRangeSets(rp) called\n";
    setRunAuxiliaryRangeSetID(ranges);
  }

  bool
  OutputModule::doEndRun(RunPrincipal const& rp, ModuleContext const& mc)
  {
    FDEBUG(2) << "endRun called\n";
    endRun(rp);
    Run const r{rp, mc};
    cet::for_all(plugins_, [&r](auto& p) { p->doEndRun(r); });
    return true;
  }

  void
  OutputModule::doWriteRun(RunPrincipal& rp)
  {
    FDEBUG(2) << "writeRun called\n";
    writeRun(rp);
  }

  void
  OutputModule::doEndJob()
  {
    endJob();
    cet::for_all(plugins_, [](auto& p) { p->doEndJob(); });
  }

  bool
  OutputModule::doOpenFile(FileBlock const& fb)
  {
    if (isFileOpen()) {
      return false;
    }
    openFile(fb);
    return true;
  }

  void
  OutputModule::doRespondToOpenInputFile(FileBlock const& fb)
  {
    respondToOpenInputFile(fb);
    unique_ptr<ResultsPrincipal> respHolder;
    ResultsPrincipal const* respPtr = fb.resultsPrincipal();
    if (respPtr == nullptr) {
      respHolder = make_unique<ResultsPrincipal>(
        ResultsAuxiliary{},
        moduleDescription().processConfiguration(),
        nullptr);
      respPtr = respHolder.get();
    }
    readResults(*respPtr);
  }

  void
  OutputModule::doRespondToCloseInputFile(FileBlock const& fb)
  {
    respondToCloseInputFile(fb);
  }

  void
  OutputModule::doRespondToOpenOutputFiles(FileBlock const& fb)
  {
    respondToOpenOutputFiles(fb);
  }

  void
  OutputModule::doRespondToCloseOutputFiles(FileBlock const& fb)
  {
    respondToCloseOutputFiles(fb);
  }

  bool
  OutputModule::doCloseFile()
  {
    if (isFileOpen()) {
      reallyCloseFile();
      return true;
    }
    return false;
  }

  void
  OutputModule::reallyCloseFile()
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
    writeFileCatalogMetadata();
    writeProductDependencies();
    finishEndFile();
    branchParents_.clear();
    branchChildren_.clear();
  }

  // Called every event (by doWriteEvent) toupdate branchParents_
  // and branchChildren_.
  void
  OutputModule::updateBranchParents(EventPrincipal& ep)
  {
    // Note: threading: We are implicitly using the Principal
    //       iterators here which iterate over the groups held
    //       by the principal, which may be updated by a producer
    //       task in another stream while we are iterating! But
    //       only for Run, SubRun, and Results principals, in the
    //       case of Event principals we arrange that no producer
    //       or filter tasks are running when we run. So since we
    //       are only called for event principals we are safe.
    //
    // Note: threading: We update branchParents_ and
    //       branchChildren_ here which must be protected if we
    //       become a stream or global module.
    //
    for (auto const& pid_and_uptr_to_grp : ep) {
      auto const& group = *pid_and_uptr_to_grp.second;
      if (group.productProvenance()) {
        ProductID const pid = pid_and_uptr_to_grp.first;
        auto iter = branchParents_.find(pid);
        if (iter == branchParents_.end()) {
          iter = branchParents_.emplace(pid, set<ParentageID>{}).first;
        }
        iter->second.insert(group.productProvenance()->parentageID());
        branchChildren_.insertEmpty(pid);
      }
    }
  }

  // Called at file close to update branchChildren_ from the accumulated
  // branchParents_.
  void
  OutputModule::fillDependencyGraph()
  {
    for (auto const& bp : branchParents_) {
      ProductID const child = bp.first;
      set<ParentageID> const& eIds = bp.second;
      for (auto const& eId : eIds) {
        Parentage par;
        if (!ParentageRegistry::get(eId, par)) {
          continue;
        }
        for (auto const& p : par.parents()) {
          branchChildren_.insertChild(p, child);
        }
      }
    }
  }

  void
  OutputModule::beginJob()
  {}

  void
  OutputModule::endJob()
  {}

  void
  OutputModule::event(EventPrincipal const&)
  {}

  void
  OutputModule::beginRun(RunPrincipal const&)
  {}

  void
  OutputModule::endRun(RunPrincipal const&)
  {}

  void
  OutputModule::beginSubRun(SubRunPrincipal const&)
  {}

  void
  OutputModule::endSubRun(SubRunPrincipal const&)
  {}

  void
  OutputModule::setRunAuxiliaryRangeSetID(RangeSet const&)
  {}

  void
  OutputModule::setSubRunAuxiliaryRangeSetID(RangeSet const&)
  {}

  void
  OutputModule::openFile(FileBlock const&)
  {}

  void
  OutputModule::respondToOpenInputFile(FileBlock const&)
  {}

  void
  OutputModule::readResults(ResultsPrincipal const&)
  {}

  void
  OutputModule::respondToCloseInputFile(FileBlock const&)
  {}

  void
  OutputModule::respondToOpenOutputFiles(FileBlock const&)
  {}

  void
  OutputModule::respondToCloseOutputFiles(FileBlock const&)
  {}

  bool
  OutputModule::isFileOpen() const
  {
    return true;
  }

  void
  OutputModule::setFileStatus(OutputFileStatus const)
  {}

  void
  OutputModule::startEndFile()
  {}

  void
  OutputModule::writeFileFormatVersion()
  {}

  void
  OutputModule::writeFileIdentifier()
  {}

  void
  OutputModule::writeFileIndex()
  {}

  void
  OutputModule::writeEventHistory()
  {}

  void
  OutputModule::writeProcessConfigurationRegistry()
  {}

  void
  OutputModule::writeProcessHistoryRegistry()
  {}

  void
  OutputModule::writeParameterSetRegistry()
  {}

  void
  OutputModule::writeBranchIDListRegistry()
  {}

  void
  OutputModule::writeParentageRegistry()
  {}

  void
  OutputModule::writeProductDescriptionRegistry()
  {}

  namespace {
    void
    collectStreamSpecificMetadata(
      vector<unique_ptr<FileCatalogMetadataPlugin>> const& plugins,
      vector<string> const& pluginNames,
      FileCatalogMetadata::collection_type& ssmd)
    {
      size_t pluginCounter = 0;
      ostringstream errors;
      for (auto& plugin : plugins) {
        FileCatalogMetadata::collection_type tmp = plugin->doProduceMetadata();
        ssmd.reserve(tmp.size() + ssmd.size());
        for (auto&& entry : tmp) {
          if (ServiceHandle<FileCatalogMetadata const> {}->wantCheckSyntax()) {
            rapidjson::Document d;
            string checkString("{ ");
            checkString +=
              cet::canonical_string(entry.first) + " : " + entry.second + " }";
            if (d.Parse(checkString.c_str()).HasParseError()) {
              auto const nSpaces = d.GetErrorOffset();
              cerr << "nSpaces = " << nSpaces << ".\n";
              errors << "OutputModule::writeCatalogMetadata():"
                     << "syntax error in metadata produced by plugin "
                     << pluginNames[pluginCounter] << ":\n"
                     << rapidjson::GetParseError_En(d.GetParseError())
                     << " Faulty key/value clause:\n"
                     << checkString << "\n"
                     << (nSpaces ? string(nSpaces, '-') : "") << "^\n";
            }
          }
          ssmd.emplace_back(move(entry));
        }
        ++pluginCounter;
      }
      auto const errMsg = errors.str();
      if (!errMsg.empty()) {
        throw Exception(errors::DataCorruption) << errMsg;
      }
    }
  } // namespace

  void
  OutputModule::writeFileCatalogMetadata()
  {
    // Obtain metadata from service for output.
    FileCatalogMetadata::collection_type md;
    ServiceHandle<FileCatalogMetadata const> {}
    ->getMetadata(md);
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
    FileCatalogMetadata::collection_type ssmd;
    collectStreamSpecificMetadata(plugins_, pluginNames_, ssmd);
    doWriteFileCatalogMetadata(md, ssmd);
  }

  void
  OutputModule::doWriteFileCatalogMetadata(
    FileCatalogMetadata::collection_type const&,
    FileCatalogMetadata::collection_type const&)
  {}

  void
  OutputModule::writeProductDependencies()
  {}

  void
  OutputModule::finishEndFile()
  {}

  OutputModule::PluginCollection_t
  OutputModule::makePlugins_(ParameterSet const& top_pset)
  {
    auto const psets = top_pset.get<vector<ParameterSet>>("FCMDPlugins", {});
    PluginCollection_t result;
    result.reserve(psets.size());
    size_t count{0};
    try {
      for (auto const& pset : psets) {
        pluginNames_.emplace_back(pset.get<string>("plugin_type"));
        auto const& libspec = pluginNames_.back();
        auto const pluginType = pluginFactory_.pluginType(libspec);
        if (pluginType !=
            cet::PluginTypeDeducer<FileCatalogMetadataPlugin>::value) {
          throw Exception(errors::Configuration, "OutputModule: ")
            << "unrecognized plugin type " << pluginType << ".\n";
        }
        result.emplace_back(
          pluginFactory_.makePlugin<unique_ptr<FileCatalogMetadataPlugin>>(
            libspec, pset));
        ++count;
      }
    }
    catch (cet::exception& e) {
      throw Exception(errors::Configuration, "OutputModule: ", e)
        << "Exception caught while processing FCMDPlugins[" << count
        << "] in module " << moduleDescription().moduleLabel() << ".\n";
    }
    return result;
  }

  int
  OutputModule::maxEvents() const
  {
    return maxEvents_;
  }

  int
  OutputModule::remainingEvents() const
  {
    return remainingEvents_;
  }

  SelectionsArray const&
  OutputModule::keptProducts() const
  {
    return keptProducts_;
  }

  bool
  OutputModule::selected(BranchDescription const& pd) const
  {
    auto const bt = pd.branchType();
    assert(groupSelector_[bt]);
    return groupSelector_[bt]->selected(pd);
  }

  std::array<bool, NumBranchTypes> const&
  OutputModule::hasNewlyDroppedBranch() const
  {
    return hasNewlyDroppedBranch_;
  }

  BranchChildren const&
  OutputModule::branchChildren() const
  {
    return branchChildren_;
  }

  bool
  OutputModule::limitReached() const
  {
    return remainingEvents_ == 0;
  }
} // namespace art
