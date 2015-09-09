#include "art/Framework/Core/RPManager.h"
#include "art/Framework/Core/ResultsProducer.h"
#include "cetlib/PluginTypeDeducer.h"

#include <memory>

namespace {
  std::size_t countProducers(art::RPManager::RPMap_t const & rpmap)
  {
    std::size_t result = 0ul;
    for (auto const & path : rpmap) {
      result += path.second.size();
    }
    return result;
  }
}

art::RPManager::
RPManager(fhicl::ParameterSet const & ps)
:
  pf_("plugin", "makeRP"),
  rpmap_(makeRPs_(ps)),
  size_(countProducers(rpmap_))
{
}

// auto
// art::RPManager::
// path(std::string const & pathName)
// -> RPPath_t &
// {
//   auto i = rpmap_.find(pathName);
//   if (i != rpmap_.end()) {
//     return i->second;
//   } else {
//     // FIXME: Throw informative exception.
//     throw Exception(errors::LogicError);
//   }
// }

// auto
// art::RPManager::
// path(std::string const & pathName) const
// -> RPPath_t const &
// {
//   auto i = rpmap_.find(pathName);
//   if (i != rpmap_.cend()) {
//     return i->second;
//   } else {
//     // FIXME: Throw informative exception.
//     throw Exception(errors::LogicError);
//   }
// }

namespace {
  std::unique_ptr<art::RPWrapperBase>
  pathLoader(cet::BasicPluginFactory & pf,
             fhicl::ParameterSet const & producers,
             std::string const & pkey)
  {
    std::unique_ptr<art::RPWrapperBase> result;
    auto const & pconfig = producers.get<fhicl::ParameterSet>(pkey);
    auto const & libspec = pconfig.get<std::string>("plugin_type");
    auto const & ptype = pf.pluginType(libspec);
    if (ptype == cet::PluginTypeDeducer<art::ResultsProducer>::value) {
      result = pf.makePlugin<std::unique_ptr<art::RPWrapperBase> >(libspec, std::cref(pconfig));
    } else {
      // FIXME: Throw informative exception.
      throw art::Exception(art::errors::Configuration);
    }
    return result;
  }
}

auto
art::RPManager::
makeRPs_(fhicl::ParameterSet const & ps)
-> decltype(rpmap_)
{
  using fhicl::ParameterSet;
  decltype(rpmap_) result;
  auto results = ps.get<ParameterSet>("results", {});
  auto producers = results.get<ParameterSet>("producers", {});
  auto results_keys =  results.get_names();
  auto producer_keys = producers.get_names();
  std::map<std::string, std::vector<std::string> > paths;

  if (producer_keys.empty()) {
    if (!results.is_empty()) {
      throw Exception(errors::Configuration)
        << "Parameter set "
        << ps.get<std::string>("module_label")
        << ".results is non-empty, but does not contain\n"
        << "a non-empty producers parameter set.\n";
    }
  } else {
    auto const path_keys_end = std::remove(results_keys.begin(), results_keys.end(), "producers");
    for (auto path_key = results_keys.begin(); path_key != path_keys_end; ++path_key) {
      if (!results.is_key_to_sequence(*path_key)) {
        throw Exception(errors::Configuration)
          << "Parameter"
          << ps.get<std::string>("module_label")
          << ".results."
          << *path_key
          << " does not describe a sequence (i.e. a path).\n";
      } else {
        paths.emplace(*path_key, results.get<std::vector<std::string>>(*path_key));
      }
    }
    if (paths.empty()) {
      paths.emplace("default_path", producers.get_names());
    }
  }
  for (auto const & path : paths) {
    auto ins_res = rpmap_.emplace(path.first, decltype(rpmap_)::mapped_type {});
    std::transform(path.second.cbegin(),
                   path.second.cend(),
                   std::back_inserter(ins_res.first->second),
                   std::bind(&pathLoader,
                             std::ref(pf_),
                             std::cref(producers),
                             std::placeholders::_1));
  }
  return result;
}
