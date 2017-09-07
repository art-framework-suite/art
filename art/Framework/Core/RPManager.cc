#include "art/Framework/Core/RPManager.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/RPWorkerT.h"
#include "art/Framework/Core/ResultsProducer.h"
#include "art/Utilities/PluginSuffixes.h"
#include "cetlib/BasicPluginFactory.h"
#include "cetlib/PluginTypeDeducer.h"
#include "fhiclcpp/ParameterSet.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace std;
using namespace literals::string_literals;
using fhicl::ParameterSet;

namespace art {

RPManager::
RPManager(fhicl::ParameterSet const& ps)
  : rpmap_{}
{
  string const omLabel(ps.get<string>("module_label"));
  auto results = ps.get<ParameterSet>("results", {});
  auto producers = results.get<ParameterSet>("producers", {});
  auto results_keys = results.get_names();
  auto producer_keys = producers.get_names();
  map<string, vector<string>> paths;
  string errMsg;
  if (producer_keys.empty() && !results.is_empty()) {
    throw Exception(errors::Configuration)
        << "Parameter set "
        << omLabel
        << ".results is non-empty, but does not contain\n"
        << "a non-empty producers parameter set.\n";
  }
  auto const path_keys_end = remove(results_keys.begin(), results_keys.end(), "producers");
  auto const& all_labels = producers.get_names();
  set<string> all_labels_set(all_labels.cbegin(), all_labels.cend());
  map<string, string> used_labels;
  for (auto path_key = results_keys.begin(); path_key != path_keys_end; ++path_key) {
    if (!results.is_key_to_sequence(*path_key)) {
      errMsg += "Parameter " + omLabel + ".results." + *path_key + " does not describe a sequence (i.e. a path).\n";
      continue;
    }
    auto path = results.get<vector<string>>(*path_key);
    int idx = 0;
    for (auto const& l : path) {
      if (all_labels_set.find(l) == all_labels_set.end()) {
        // Bad label
        errMsg += omLabel + ".results."s + *path_key + '[' + to_string(idx) + "] ("s + l + ')' + " is not defined in "s + omLabel +
                  ".results.producers.\n"s;
        ++idx;
        continue;
      }
      auto const ans = used_labels.emplace(l, *path_key);
      if (!ans.second) {
        // Duplicate
        errMsg += omLabel + ".results." + *path_key + '[' + to_string(idx) + "] (" + l + ')' +
                  " is a duplicate: previously used in path " + ans.first->second + ".\n";
      }
      ++idx;
    }
    if (errMsg.empty()) {
      paths.emplace(*path_key, move(path));
    }
  }
  if (paths.empty() && (errMsg.empty())) {
    paths.emplace("default_path", all_labels);
  }
  if (!errMsg.empty()) {
    throw Exception(errors::Configuration)
        << "Errors encountered while configuring ResultsProducers:\n"
        << errMsg;
  }
  cet::BasicPluginFactory pf{Suffixes::plugin(), "makeRP"};
  for (auto const& path : paths) {
    auto ins_res = rpmap_.emplace(path.first, vector<unique_ptr<RPWorker>>{});
    transform(path.second.cbegin(), path.second.cend(),
              back_inserter(ins_res.first->second),
              [&pf, &producers, &omLabel, &errMsg](string const& pkey) {
                unique_ptr<RPWorker> result;
                auto const& pconfig = producers.get<ParameterSet>(pkey);
                auto libspec = pconfig.get<string>("plugin_type");
                auto const& ptype = pf.pluginType(libspec);
                if (ptype == cet::PluginTypeDeducer<ResultsProducer>::value) {
                  result = pf.makePlugin<unique_ptr<RPWorker>, RPParams const&, ParameterSet const&>(libspec,
                                          {pconfig.id(), libspec, pkey}, pconfig);
                }
                else {
                  errMsg += "Parameter set " + omLabel + ".results." + pkey + " specifies a plugin " + libspec +
                            "\n  which is not of required type ResultsProducer.\n";
                }
                return result;
              });
  }
  if (!errMsg.empty()) {
    throw Exception(errors::Configuration)
        << "Errors encountered while instantiating ResultsProducers:\n"
        << errMsg;
  }
}

void
RPManager::
for_each_RPWorker(on_rpworker_t wfunc)
{
  for (auto& path : rpmap_) {
    for (auto& w : path.second) {
      wfunc(*w);
    }
  }
}

} // namespace art

