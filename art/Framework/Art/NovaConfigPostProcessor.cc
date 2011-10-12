#include "art/Framework/Art/NovaConfigPostProcessor.h"

#include "art/Utilities/ensureTable.h"
#include "boost/any.hpp"
#include "cetlib/canonical_string.h"
#include "cetlib/exception.h"
#include "cpp0x/algorithm"
#include "fhiclcpp/exception.h"
#include "fhiclcpp/coding.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/extended_value.h"

#include <iterator>
#include <iostream>
#include <string>
#include <vector>

using namespace fhicl;

typedef extended_value::table_t table_t;
typedef extended_value::sequence_t sequence_t;

NovaConfigPostProcessor::NovaConfigPostProcessor()
  :
  sources_(),
  tFileName_(),
  output_(),
  nevts_(),
  startEvt_(),
  skipEvts_(),
  trace_(),
  memcheck_(),
  wantNevts_(false),
  wantStartEvt_(false),
  wantSkipEvts_(false),
  wantTrace_(false),
  wantMemcheck_(false)
{
}

void NovaConfigPostProcessor::apply(intermediate_table & raw_config) const
{
  applySource(raw_config);
  applyOutput(raw_config);
  applyTFileName(raw_config);
  applyTrace(raw_config);
  applyMemcheck(raw_config);
}

void NovaConfigPostProcessor::sources(std::vector<std::string> const & sources)
{
  std::copy(sources.begin(),
            sources.end(),
            std::back_inserter(sources_));
}

void NovaConfigPostProcessor::tFileName(std::string const & tFileName)
{
  tFileName_ = fhicl::detail::encode(tFileName);
}

void NovaConfigPostProcessor::output(std::string const & output)
{
  output_ = fhicl::detail::encode(output);
}

void NovaConfigPostProcessor::
applySource(intermediate_table & raw_config) const
{
  if ((sources_.size() == 0) &&
      !(wantNevts_ || wantStartEvt_ || wantSkipEvts_)) { return; }
  if (!raw_config.exists("source.module_type")) {
    raw_config["source.module_type"] =
      extended_value(false, STRING, fhicl::detail::encode("RootInput"));
  }
  if (sources_.size() > 0) {
    size_t count = 0;
    for (std::vector<std::string>::const_iterator
         it = sources_.begin(),
         end_iter =  sources_.end();
         it != end_iter;
         ++it, ++count) {
      std::ostringstream loc;
      loc << "source.fileNames[" << count << "]";
      raw_config[loc.str()] =
        extended_value(false, STRING, fhicl::detail::encode(*it));
    }
  }
  if (wantNevts_) raw_config["source.maxEvents"] =
      extended_value(false, NUMBER, fhicl::detail::encode(nevts_));
  if (wantStartEvt_) raw_config["source.firstEvent"] =
      extended_value(false, NUMBER, fhicl::detail::encode(startEvt_));
  if (wantSkipEvts_) raw_config["source.skipEvents"] =
      extended_value(false, NUMBER, fhicl::detail::encode(skipEvts_));
}

void NovaConfigPostProcessor::
applyOutput(intermediate_table & raw_config) const
{
  if (output_.empty()) { return; }
  bool need_end_paths_entry = false;
  art::ensureTable(raw_config, "outputs");
  std::string out_table_name;
  table_t & outputs_table =
    boost::any_cast<table_t &>(raw_config["outputs"].value);
  if (outputs_table.size() > 1) {
    throw cet::exception("BAD_OUTPUT_CONFIG")
        << "Output configuration is ambiguous: configuration has "
        << "multiple output modules. Cannot decide where to add "
        << "specified output filename "
        << output_
        << ".";
  }
  if (outputs_table.empty()) {
    // Creating a table named, "out."
    out_table_name = "out";
    need_end_paths_entry = true;
  }
  else {
    out_table_name = outputs_table.begin()->first;
  }
  std::string out_table_path = "outputs.";
  out_table_path += out_table_name;
  // Insert / overwrite fileName spec.
  raw_config[out_table_path + ".fileName"] =
    extended_value(false, STRING, fhicl::detail::encode(output_));
  // Make sure we have a defined module_type.
  if (!raw_config.exists(out_table_path + ".module_type")) {
    raw_config[out_table_path + ".module_type"] =
      extended_value(false, STRING, fhicl::detail::encode("RootOutput"));
  }
  if (need_end_paths_entry) {
    // If we created a new output module config, we need to make a
    // path for it and add it to end paths. We will *not* detect the
    // case where an *existing* output module config is not referenced
    // in a path.
    art::ensureTable(raw_config, "physics");
    table_t & physics_table =
      boost::any_cast<table_t &>(raw_config["physics"].value);
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
    raw_config[std::string("physics.") + end_path + "[0]"] =
      extended_value(false, STRING, fhicl::detail::encode(out_table_name));
    // Add it to the end_paths list.
    if (!raw_config.exists("physics.end_paths")) {
      // end_paths doesn't already exist
      raw_config["physics.end_paths[0]"] =
        extended_value(false, STRING, fhicl::detail::encode(end_path));
    }
    else {
      try {
        boost::any_cast<sequence_t &>(raw_config["physics.end_paths"].value).
        push_back(extended_value(false,
                                 STRING,
                                 fhicl::detail::encode(end_path)));
      }
      catch (boost::bad_any_cast const & e) {
        throw cet::exception(std::string("BAD_OUTPUT_CONFIG"))
            << "Configuration item \"physics.end_paths\" exists but is not a sequence.\n";
      }
    }
  }
}

void NovaConfigPostProcessor::
applyTFileName(intermediate_table & raw_config) const
{
  std::string tFileName(tFileName_);
  try {
    if (tFileName.empty() &&
        raw_config.exists("services.TFileService") &&
        raw_config["services.TFileService"].is_a(TABLE) &&
        boost::any_cast<table_t &>(raw_config["services.TFileService"].value).empty()) {
      tFileName = "histo.root";
    }
  }
  catch (exception const & e) {
    // Ignore
  }
  if (!tFileName.empty()) {
    raw_config["services.TFileService.fileName"] =
      extended_value(false, STRING, fhicl::detail::encode(tFileName));
  }
}

void NovaConfigPostProcessor::
applyTrace(intermediate_table & raw_config) const
{
  if (wantTrace_) raw_config["services.scheduler.wantTracer"] =
      extended_value(false, BOOL, fhicl::detail::encode(trace_));
}

void NovaConfigPostProcessor::
applyMemcheck(intermediate_table & raw_config) const
{
  if (wantMemcheck_)  {
    if (memcheck_) {
      if (!raw_config.exists("services.SimpleMemoryCheck")) {
        // If it doesn't exist, make one.
        raw_config["services.SimpleMemoryCheck"] =
          extended_value(false, TABLE, table_t());
      }
    }
    else if (raw_config.exists("services.SimpleMemoryCheck")) {
      // It exists: remove it.
      table_t & s = boost::any_cast<table_t &>(raw_config["services"].value);
      s.erase(s.find("SimpleMemoryCheck"));
    }
  }
}
