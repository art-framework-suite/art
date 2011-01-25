#include "NovaConfigPostProcessor.h"

#include "boost/any.hpp"
#include "cetlib/canonical_string.h"
#include "cetlib/exception.h"
#include "fhiclcpp/exception.h"
#include "fhiclcpp/coding.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/extended_value.h"

#include <iostream>
#include <string>
#include <vector>

using namespace fhicl;

NovaConfigPostProcessor::NovaConfigPostProcessor()
   :
   source_(),
   tFileName_(),
   output_(),
   nevts_(),
   startEvt_(),
   skipEvts_(),
   trace_(),
   wantNevts_(false),
   wantStartEvt_(false),
   wantSkipEvts_(false),
   wantTrace_(false)
{
}

void NovaConfigPostProcessor::apply(intermediate_table &raw_config) const {
   applySource(raw_config);
   applyOutput(raw_config);
   applyTFileName(raw_config);
   applyTrace(raw_config);
}

void NovaConfigPostProcessor::source(std::string const &source) {
   source_ = fhicl::detail::encode(source);
}

void NovaConfigPostProcessor::tFileName(std::string const &tFileName) {
   tFileName_ = fhicl::detail::encode(tFileName);
}

void NovaConfigPostProcessor::output(std::string const &output) {
   output_ = fhicl::detail::encode(output);
}

void NovaConfigPostProcessor::
applySource(intermediate_table &raw_config) const {
   if (source_.empty() &&
       !(wantNevts_ || wantStartEvt_ || wantSkipEvts_)) return;
   extended_value::table_t source_table;
   try {
      source_table = raw_config.find("source");
   }
   catch (exception &e) {
      if (e.categoryCode() == cant_find) {
         // Ignore
      } else {
         throw;
      }
   }
   if (source_table.find("module_type") == source_table.end()) {
      source_table["module_type"] = extended_value(false, STRING, fhicl::detail::encode("RootInput"));
   }
   if (!source_.empty()) {
      extended_value::sequence_t fileNames;
      fileNames.push_back(extended_value(false, STRING, fhicl::detail::encode(source_)));
      source_table["fileNames"] = extended_value(false, SEQUENCE, fileNames);
   }
   if (wantNevts_) {
      source_table["maxEvents"] = extended_value(false, NUMBER, fhicl::detail::encode(nevts_));
   }
   if (wantStartEvt_) {
      source_table["firstEvent"] = extended_value(false, NUMBER, fhicl::detail::encode(startEvt_));
   }
   if (wantSkipEvts_) {
      source_table["skipEvents"] = extended_value(false, NUMBER, fhicl::detail::encode(skipEvts_));
   }
   raw_config.insert("source", extended_value(false, TABLE, source_table));
}

void NovaConfigPostProcessor::
applyOutput(intermediate_table &raw_config) const {
   if (output_.empty()) return;
   bool need_end_paths_entry = false;
   extended_value::table_t outputs_table;
   try {
      outputs_table = raw_config.find("outputs");
   }
   catch (exception &e) {
      if (e.categoryCode() == cant_find) {
         // Ignore
      } else {
         throw;
      }
   }
   extended_value::table_t out_table;
   std::string out_table_name;
   if (outputs_table.empty()) {
      out_table_name = "out";
   } else if (outputs_table.size() > 1) {
      throw cet::exception("BAD_OUTPUT_CONFIG")
         << "Output configuration is ambiguous: configuration has "
         << "multiple output modules. Cannot decide where to add "
         << "specified output filename "
         << output_
         << ".";
   } else {
      out_table_name = outputs_table.begin()->first;
      out_table = outputs_table.begin()->second;
   }
   if (out_table.empty()) {
      // Fill a basic output module config
      out_table["module_type"] = extended_value(false, STRING, fhicl::detail::encode("RootOutput"));
      need_end_paths_entry = true;
   }
   // Fill in the file name.
   out_table["fileName"] = extended_value(false, STRING, fhicl::detail::encode(output_));
   // Put back into the outputs table.
   outputs_table[out_table_name] = extended_value(false, TABLE, out_table);
   // Put outputs table back into config.
   raw_config.insert("outputs", false, TABLE, outputs_table);
   if (need_end_paths_entry) {
      // If we created a new output module config, we need to make a
      // path for it and add it to end paths. We will *not* detect the
      // case where an *existing* output module config is not referenced
      // in a path.
      extended_value::table_t physics_table;
      try {
         physics_table = raw_config.find("physics");
      }
      catch (exception &e) {
         if (e.categoryCode() == cant_find) {
            // Ignore
         } else {
            throw;
         }
      }
      std::string new_path = "injected_end_path_";
      for (size_t
              i = 1,
              n = physics_table.size() + 2;
           i < n;
           ++i) {
         std::ostringstream os;
         os << new_path;
         os << i;
         if (physics_table.find(os.str()) == physics_table.end()) {
            new_path = os.str();
            break;
         }
      }
      // Not possible to get to here without having a good path name.
      extended_value::sequence_t end_path;
      end_path.push_back(extended_value(false, STRING, fhicl::detail::encode(out_table_name)));
      physics_table[new_path] = extended_value(false, SEQUENCE, end_path);
      extended_value::sequence_t end_paths;
      if (physics_table.find("end_paths") != physics_table.end()) {
         end_paths = physics_table["end_paths"];
      }
      end_paths.push_back(extended_value(false, STRING, fhicl::detail::encode(new_path)));
      physics_table["end_paths"] = extended_value(false, SEQUENCE, end_paths);
      raw_config.insert("physics", false, TABLE, physics_table);
   }
}

void NovaConfigPostProcessor::
applyTFileName(intermediate_table &raw_config) const {
   std::string tFileName(tFileName_);

   extended_value::table_t services_table;
   try {
      services_table = raw_config.find("services");
   }
   catch (exception &e) {
      if (e.categoryCode() == cant_find) {
         // Ignore
      } else {
         throw;
      }
   }

   extended_value::table_t::iterator t_iter = services_table.find("TFileService");
   extended_value::table_t tFileService_table;
   if (t_iter != services_table.end()) {
      tFileService_table = t_iter->second;
      if (tFileService_table.empty() && tFileName.empty()) {
         // Only if TFileService clause is specified but empty
         // *and* we haven't specified a file name.
         tFileName = "histo.root";
      }
   }
   if (tFileName.empty()) return;
   tFileService_table["fileName"] = extended_value(false, STRING, fhicl::detail::encode(tFileName));
   services_table["TFileService"] = extended_value(false, TABLE, tFileService_table);
   raw_config.insert("services", extended_value(false, TABLE, services_table));
}

void NovaConfigPostProcessor::
applyTrace(intermediate_table &raw_config) const {
   if (!wantTrace_) return;
   extended_value::table_t services_table;
   try {
      services_table = raw_config.find("services");
   }
   catch (exception &e) {
      if (e.categoryCode() == cant_find) {
         // Ignore
      } else {
         throw;
      }
   }
   extended_value::table_t::iterator t_iter = services_table.find("scheduler");
   extended_value::table_t  scheduler_table;
   if (t_iter != services_table.end()) scheduler_table = t_iter->second;
   scheduler_table["wantTracer"] = extended_value(false, BOOL, fhicl::detail::encode(trace_));
   services_table["scheduler"] = extended_value(false, TABLE, scheduler_table);
   raw_config.insert("services", extended_value(false, TABLE, services_table));
}
