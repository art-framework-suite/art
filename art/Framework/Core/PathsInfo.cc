#include "art/Framework/Core/PathsInfo.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Utilities/bold_fontify.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "cetlib/HorizontalRule.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/detail/validationException.h"

art::PathsInfo::PathsInfo(std::size_t const numPaths,
                          detail::ModuleFactory& factory,
                          fhicl::ParameterSet const& procPS,
                          MasterProductRegistry& preg,
                          ProductDescriptions& productsToProduce,
                          ActionTable& actions,
                          ActivityRegistry& areg) :
  pathResults_{numPaths},
  fact_{factory},
  procPS_{procPS},
  preg_{preg},
  productsToProduce_{productsToProduce},
  exceptActions_{actions},
  areg_{areg}
{}

// Precondition: !modInfos.empty();
void
art::PathsInfo::makeAndAppendPath(std::string const& pathName,
                                  ModInfos const& modInfos,
                                  bool const trigResultsNeeded)
{
  assert(!modInfos.empty());
  std::vector<WorkerInPath> pathWorkers;
  for (auto const& mci : modInfos) {
    makeWorker_(mci, pathWorkers);
  }

  if (!configErrMsgs_.empty()) {
    constexpr cet::HorizontalRule rule{100};
    std::ostringstream err_msg;
    err_msg << "\n"
            << rule('=')
            << "\n\n"
            << "!! The following modules have been misconfigured: !!"
            << "\n";
    for (auto const& err : configErrMsgs_) {
      err_msg << "\n"
              << rule('-')
              << "\n"
              << err;
    }
    err_msg << "\n"
            << rule('=')
            << "\n\n";

    throw art::Exception(art::errors::Configuration) << err_msg.str();
  }

  cet::exempt_ptr<HLTGlobalStatus> pathResults {trigResultsNeeded ? &pathResults_ : nullptr};
  auto const bit_position_for_new_path = pathPtrs_.size();
  auto path = std::make_unique<art::Path>(bit_position_for_new_path,
                                          pathName,
                                          std::move(pathWorkers),
                                          std::move(pathResults),
                                          exceptActions_,
                                          areg_,
                                          is_observer(modInfos.front().moduleConfigInfo().moduleType()));
  pathPtrs_.push_back(std::move(path));
}

void
art::PathsInfo::makeWorker_(detail::ModuleInPathInfo const& mipi,
                            std::vector<WorkerInPath>& pathWorkers)
{
  auto w = makeWorker_(mipi.moduleConfigInfo());
  pathWorkers.emplace_back(w, mipi.filterAction());
}

cet::exempt_ptr<art::Worker>
art::PathsInfo::makeWorker_(detail::ModuleConfigInfo const& mci)
{
  auto it = workers_.find(mci.label());
  if (it == workers_.end()) { // Need worker.
    auto moduleConfig = procPS_.get<fhicl::ParameterSet>(mci.configPath() + '.' + mci.label());
    WorkerParams const p{procPS_,
                         moduleConfig,
                         preg_,
                         productsToProduce_,
                         exceptActions_,
                         ServiceHandle<TriggerNamesService const>{}->getProcessName()};
    ModuleDescription const md{moduleConfig.id(),
                               p.pset_.get<std::string>("module_type"),
                               p.pset_.get<std::string>("module_label"),
                               ProcessConfiguration{p.processName_,
                                                    procPS_.id(),
                                                    getReleaseVersion()}};
    areg_.sPreModuleConstruction.invoke(md);
    try {
      auto worker = fact_.makeWorker(p, md);
      areg_.sPostModuleConstruction.invoke(md);
      it = workers_.emplace(mci.label(), std::move(worker)).first;
      it->second->setActivityRegistry(&areg_);
    }
    catch (fhicl::detail::validationException const& e) {
      std::ostringstream err_stream;
      err_stream << "\n\nModule label: " << detail::bold_fontify(md.moduleLabel())
                 <<   "\nmodule_type : " << detail::bold_fontify(md.moduleName())
                 << "\n\n" << e.what();
      configErrMsgs_.push_back(err_stream.str());
    }
  }
  return cet::exempt_ptr<Worker>{it->second.get()};
}
