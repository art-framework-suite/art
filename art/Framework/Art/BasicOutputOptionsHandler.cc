#include "art/Framework/Art/BasicOutputOptionsHandler.h"

#include "art/Framework/Art/detail/AllowedConfiguration.h"
#include "art/Framework/Art/detail/exists_outside_prolog.h"
#include "art/Framework/Art/detail/fhicl_key.h"
#include "art/Framework/Art/detail/get_LibraryInfoCollection.h"
#include "art/Utilities/PluginSuffixes.h"
#include "art/Utilities/ensureTable.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/canonical_string.h"
#include "fhiclcpp/coding.h"
#include "fhiclcpp/extended_value.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/parse.h"

#include <iostream>
#include <regex>
#include <string>
#include <vector>

using namespace std::string_literals;
using art::detail::fhicl_key;

using table_t = fhicl::extended_value::table_t;
using sequence_t = fhicl::extended_value::sequence_t;
using art::detail::exists_outside_prolog;

using stringvec = std::vector<std::string>;

art::BasicOutputOptionsHandler::BasicOutputOptionsHandler(
  bpo::options_description& desc)
{
  bpo::options_description output_options{"Output options"};
  // clang-format off
  output_options.add_options()
    ("TFileName,T", bpo::value<std::string>(), "File name for TFileService.")
    ("tmpdir",
       bpo::value<std::string>(&tmpDir_),
       "Temporary directory for in-progress output files (defaults to directory "
       "of specified output file names).")
    ("tmpDir", bpo::value<std::string>(&tmpDir_), "Synonym for --tmpdir.")
    ("output,o",
       bpo::value<stringvec>()->composing(),
       "Event output stream file (optionally specify stream with "
       "stream-label:fileName in which case multiples are OK).")
    ("no-output", "Disable all output streams.");
  // clang-format on
  desc.add(output_options);
}

int
art::BasicOutputOptionsHandler::doCheckOptions(bpo::variables_map const&)
{
  return 0;
}

namespace {
  using art::ensureTable;

  // Remove any occurrences of a single module label (key) from the
  // fully qualified sequence parameter pathName in raw_config.
  // Returns true if path exists and is now empty.
  bool
  maybeRemoveFromPath(fhicl::intermediate_table& raw_config,
                      std::string const& pathName,
                      std::string const& key)
  {
    bool result = false;
    if (exists_outside_prolog(raw_config, pathName)) {
      sequence_t& path = raw_config.get<sequence_t&>(pathName);
      auto path_end = std::remove_if(
        path.begin(), path.end(), [&key](fhicl::extended_value const& s) {
          return cet::canonical_string(key) == fhicl::extended_value::atom_t(s);
        });
      if (path_end != path.end()) { // Shrunk!
        path.resize(std::distance(path.begin(), path_end));
      }
      if (path.empty()) {
        result = true;
      }
    }
    return result;
  }

  // Remove a given key from all paths.
  void
  removeFromEndPaths(fhicl::intermediate_table& raw_config,
                     std::string const& key)
  {
    std::string const& physicsKey{"physics"};
    if (!exists_outside_prolog(raw_config, physicsKey)) {
      return;
    }
    auto& physics_table(raw_config.get<table_t&>(physicsKey));
    std::vector<std::string> const ignoredParameters(
      {"analyzers", "filters", "producers", "end_paths", "trigger_paths"});
    auto i = physics_table.begin();
    auto e = physics_table.end();
    for (; i != e;) {
      if (std::find(ignoredParameters.cbegin(),
                    ignoredParameters.cend(),
                    i->first) == ignoredParameters.end() &&
          i->second.is_a(fhicl::SEQUENCE) &&
          maybeRemoveFromPath(raw_config, physicsKey + '.' + i->first, key)) {
        // Remove empty path from end_paths.
        maybeRemoveFromPath(raw_config, physicsKey + ".end_paths", i->first);
        // Remove empty path from table.
        i = physics_table.erase(i);
      } else {
        ++i;
      }
    }
  }

  void
  processSpecifiedOutputs(fhicl::intermediate_table& raw_config,
                          stringvec outputs)
  {
    auto const b = outputs.begin(), e = outputs.end();
    for (auto i = b; i != e; ++i) {
      auto& output = *i;
      bool new_path_entry(false);
      auto const outputsKey = "outputs"s;
      ensureTable(raw_config, outputsKey);

      auto& outputs_table(raw_config.get<table_t&>(outputsKey));
      std::smatch splitResult;
      static std::regex const streamSplitter("([[:alnum:]]+):(?:/[^/]|[^/]).*");
      std::string streamName;
      if (std::regex_match(output, splitResult, streamSplitter)) {
        streamName = splitResult[1];
        output.erase(0ull, streamName.size() + 1ull);
      } else if (b != i) {
        throw art::Exception(art::errors::Configuration)
          << "While processing specified output " << output
          << ": only the first specified output may omit the stream "
             "specification\n"
             "(\"label:fileName\").\n";
      } else if (outputs_table.size() == 1ull) {
        streamName = outputs_table.cbegin()->first;
      } else {
        streamName = "out"s;
      }

      if (outputs_table.size() > 1ull && splitResult.empty()) {
        throw art::Exception(art::errors::Configuration)
          << "Output configuration is ambiguous: configuration has "
          << "multiple output modules. Cannot decide where to add "
          << "specified output filename " << output
          << ".\nUse stream-specification (\"label:fileName\") to resolve the "
             "ambiguity.";
      }

      if (outputs_table.find(streamName) == outputs_table.cend()) {
        new_path_entry = true;
        std::string const defaultOutputModule =
          art::detail::get_LibraryInfoCollection(art::Suffixes::module(),
                                                 "RootOutput")
              .empty() ?
            "FileDumperOutput" :
            "RootOutput";
        raw_config.put(outputsKey + '.' + streamName + '.' + "module_type",
                       defaultOutputModule);
      }
      std::string out_table_path(outputsKey);
      out_table_path += '.' + streamName;
      raw_config.put(out_table_path + ".fileName", output);
      if (new_path_entry) {
        // If we created a new output module config, we need to make a
        // path for it and add it to end paths. We will *not* detect the
        // case where an *existing* output module config is not
        // referenced in a path.
        ensureTable(raw_config, "physics");
        auto& physics_table = raw_config.get<table_t&>("physics");
        // Find an unique name for the end_path into which we'll insert
        // our new module.
        std::string end_path = "injected_end_path_";
        size_t n = physics_table.size() + 2;
        for (size_t i = 1; i < n; ++i) {
          std::ostringstream os;
          os << end_path << i;
          if (physics_table.find(os.str()) == physics_table.end()) {
            end_path = os.str();
            break;
          }
        }
        // Make our end_path with the output module label in it.
        raw_config.put("physics."s + end_path + "[0]", streamName);
        // Add it to the end_paths list.
        auto const key = "physics.end_paths"s;
        if (exists_outside_prolog(raw_config, key)) {
          size_t const index =
            raw_config.get<sequence_t&>("physics.end_paths").size();
          raw_config.put("physics.end_paths["s + std::to_string(index) + ']',
                         end_path);
        }
      }
    }
  }

  void
  processFileOutputOptions(bpo::variables_map const& vm,
                           fhicl::intermediate_table& raw_config)
  {
    // File output.
    if (vm.count("no-output") == 1) {
      if (vm.count("output")) {
        throw art::Exception(art::errors::Configuration)
          << "Output configuration is ambiguous: command-line specifies "
          << "--output and --no-output simultaneously.";
      }
      std::string const& key{"outputs"};
      if (exists_outside_prolog(raw_config, key)) {
        auto& outputs_table(raw_config.get<table_t&>(key));
        // No outputs.
        for (auto const& p : outputs_table) {
          removeFromEndPaths(raw_config, p.first);
        }
        raw_config.erase(key);
      }
    } else if (vm.count("output") == 1) {
      processSpecifiedOutputs(raw_config, vm["output"].as<stringvec>());
    }
  }
} // namespace

int
art::BasicOutputOptionsHandler::doProcessOptions(
  bpo::variables_map const& vm,
  fhicl::intermediate_table& raw_config)
{
  // TFileService output.
  if (vm.count("TFileName") == 1) {
    std::string tFileName(vm["TFileName"].as<std::string>());
    std::string const& key{"services.TFileService"};
    if (tFileName.empty() && detail::exists_outside_prolog(raw_config, key) &&
        raw_config.get<table_t const&>(key).empty()) {
      tFileName = "hist.root";
    }
    if (!tFileName.empty()) {
      raw_config.put(key + ".fileName", tFileName);
    }
  }
  // Output stream options.
  processFileOutputOptions(vm, raw_config);
  // tmpDir option for TFileService and output streams.
  if (!tmpDir_.empty()) {
    std::string const& tfile_key = fhicl_key("services", "TFileService");
    if (detail::exists_outside_prolog(raw_config, tfile_key)) {
      auto has_tmpDir [[maybe_unused]] =
        detail::supports_key(Suffixes::service(), "TFileService", "tmpDir");
      assert(has_tmpDir); // Should never happen.
      raw_config.put(fhicl_key(tfile_key, "tmpDir"), tmpDir_);
    }

    // Inject tmpDir for output modules that support the 'tmpDir' parameter.
    std::string const outputs_stem{"outputs"};
    if (detail::exists_outside_prolog(raw_config, outputs_stem)) {
      auto const& table = raw_config.get<table_t const&>(outputs_stem);
      for (auto const& output : table) {
        auto const& module_label = fhicl_key(outputs_stem, output.first);
        auto const& module_type = fhicl_key(module_label, "module_type");
        if (!detail::exists_outside_prolog(raw_config, module_type))
          continue;

        auto const& spec = raw_config.get<std::string>(module_type);
        if (!detail::supports_key(Suffixes::module(), spec, "tmpDir"))
          continue;

        raw_config.put(fhicl_key(module_label, "tmpDir"), tmpDir_);
      }
    }
  }

  return 0;
}
