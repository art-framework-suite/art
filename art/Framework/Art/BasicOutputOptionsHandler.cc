#include "art/Framework/Art/BasicOutputOptionsHandler.h"

#include "art/Utilities/ensureTable.h"
#include "cetlib/canonical_string.h"
#include "fhiclcpp/coding.h"
#include "fhiclcpp/extended_value.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/parse.h"

#include <iostream>
#include <string>

using namespace std::string_literals;

namespace {
  using table_t = fhicl::extended_value::table_t;
  using sequence_t = fhicl::extended_value::sequence_t;
}

art::BasicOutputOptionsHandler::
BasicOutputOptionsHandler(bpo::options_description & desc)
{
  desc.add_options()
    ("TFileName,T", bpo::value<std::string>(), "File name for TFileService.")
    ("tmpdir", bpo::value<std::string>(), "Temporary directory for in-progress output files (defaults to directory of specified output file names).")
    ("output,o", bpo::value<std::string>(), "Event output stream file.")
    ("no-output", "Disable all output streams.")
  ;
}

int
art::BasicOutputOptionsHandler::
doCheckOptions(bpo::variables_map const &)
{
  return 0;
}


namespace {
  using art::ensureTable;

  // Remove any occurrences of a single module label (key) from the
  // fully qualified sequence parameter pathName in raw_config.
  // Returns true if path exists and is now empty.
  bool
  maybeRemoveFromPath(fhicl::intermediate_table & raw_config,
                      std::string const & pathName,
                      std::string const & key)
  {
    bool result = false;
    if (raw_config.exists(pathName)) {
      sequence_t & path = raw_config.get<sequence_t &>(pathName);
      auto path_end =
        std::remove_if(path.begin(),
                       path.end(),
                       [&key](fhicl::extended_value const & s)
                       { return cet::canonical_string(key) == fhicl::extended_value::atom_t(s); });
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
  removeFromEndPaths(fhicl::intermediate_table & raw_config,
                     std::string const & key)
  {
    if (!raw_config.exists("physics")) {
      return;
    }
    auto & physics_table(raw_config.get<table_t &>("physics"));
    std::vector<std::string> const
      ignoredParameters({"analyzers",
            "filters",
            "producers",
            "end_paths",
            "trigger_paths"});
    auto i = physics_table.begin();
    auto e = physics_table.end();
    for (; i != e;) {
      if (std::find(ignoredParameters.cbegin(),
                    ignoredParameters.cend(),
                    i->first) == ignoredParameters.end() &&
          i->second.is_a(fhicl::SEQUENCE) &&
          maybeRemoveFromPath(raw_config, std::string("physics.") + i->first, key)) {
        // Remove empty path from end_paths.
        maybeRemoveFromPath(raw_config, "physics.end_paths", i->first);
        // Remove empty path from table.
        i = physics_table.erase(i);
      } else {
        ++i;
      }
    }
  }

  void
  processOutputOption(fhicl::intermediate_table & raw_config,
                      std::string const & output)
  {
    bool const want_output = (output != "/dev/null");
    bool new_path_entry(false);
    if (want_output) {
      ensureTable(raw_config, "outputs");
    } else if (!raw_config.exists("outputs")) {
      // Nothing to do.
      return;
    }
    auto & outputs_table(raw_config.get<table_t &>("outputs"));
    if (outputs_table.empty()) {
      if (want_output) {
        new_path_entry = true;
        raw_config.put("outputs.out.module_type", "RootOutput");
      } else {
        // Nothing to do.
        return;
      }
    }
    else if (outputs_table.size() > 1) {
      throw cet::exception("BAD_OUTPUT_CONFIG")
        << "Output configuration is ambiguous: configuration has "
        << "multiple output modules. Cannot decide where to add "
        << "specified output filename "
        << output
        << ".";
    }
    else {
      // Empty.
    }
    std::string out_table_name(outputs_table.begin()->first);
    assert(!out_table_name.empty());
    if (!want_output) {
      // Remove this from paths.
      removeFromEndPaths(raw_config, out_table_name);
      // Remove outputs entrirely.
      raw_config.erase("outputs");
      return;
    }
    std::string out_table_path("outputs");
    out_table_path += "." + out_table_name;
    raw_config.put(out_table_path + ".fileName", output);
    if (new_path_entry) {
      // If we created a new output module config, we need to make a
      // path for it and add it to end paths. We will *not* detect the
      // case where an *existing* output module config is not
      // referenced in a path.
      ensureTable(raw_config, "physics");
      auto & physics_table = raw_config.get<table_t &>("physics");
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
      raw_config.put("physics."s + end_path + "[0]", out_table_name);
      // Add it to the end_paths list.
      if ( raw_config.exists("physics.end_paths") ) {
        size_t const index = raw_config.get<sequence_t &>("physics.end_paths").size();
        raw_config.put("physics.end_paths["s + std::to_string(index) + "]", end_path);
      }
    }
  }

  void
  processFileOutputOptions(bpo::variables_map const & vm,
                           fhicl::intermediate_table & raw_config)
  {
    // File output.
    if (vm.count("no-output") == 1) {
      if (vm.count("output")) {
        throw cet::exception("BAD_OUTPUT_CONFIG")
          << "Output configuration is ambiguous: command-line specifies"
          << "--output and --no-output simultaneously.";
      }
      if (raw_config.exists("outputs")) {
        auto & outputs_table(raw_config.get<table_t &>("outputs"));
        // No outputs.
        for (auto const & p : outputs_table) {
          removeFromEndPaths(raw_config, p.first);
        }
        raw_config.erase("outputs");
      }
    }
    else if (vm.count("output") == 1) {
      processOutputOption(raw_config, vm["output"].as<std::string>());
    }
  }
}

int
art::BasicOutputOptionsHandler::
doProcessOptions(bpo::variables_map const & vm,
                 fhicl::intermediate_table & raw_config)
{
  // TFileService output.
  if (vm.count("TFileName") == 1) {
    std::string tFileName(vm["TFileName"].as<std::string>());
    if (tFileName.empty() &&
        raw_config.exists("services.TFileService") &&
        raw_config.get<table_t const &>("services.TFileService").empty()) {
      tFileName = "hist.root";
    }
    if (!tFileName.empty()) {
      raw_config.put("services.TFileService.fileName",
                     tFileName);
    }
  }
  // Output stream options.
  processFileOutputOptions(vm, raw_config);
  // tmpDir option for TFileService and output streams.
  if (vm.count("tmpdir") == 1) {
    auto tmpDir = vm["tmpdir"].as<std::string>();
    std::string const outputs_stem("outputs.");
    if (raw_config.exists("services.TFileService")) {
      raw_config.put("services.TFileService.tmpDir", tmpDir);
    }
    if (raw_config.exists("outputs")) {
      auto const & table = raw_config.get<table_t const &>("outputs");
      for (auto const & output : table) {
        if (raw_config.exists(outputs_stem + output.first + ".module_type")) {
          // Inject tmpDir into the module configuration.
          raw_config.put(outputs_stem + output.first + ".tmpDir", tmpDir);
        }
      }
    }
  }

  return 0;
}
