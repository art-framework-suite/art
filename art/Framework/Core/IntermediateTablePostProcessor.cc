#include "art/Framework/Core/IntermediateTablePostProcessor.h"

#include "boost/any.hpp"
#include "cetlib/canonical_string.h"
#include "cetlib/exception.h"
#include "fhiclcpp/coding.h"
#include "fhiclcpp/exception.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/extended_value.h"

#include <iostream>
#include <string>
#include <vector>

using namespace art;
using namespace fhicl;

typedef extended_value::table_t table_t;
typedef extended_value::sequence_t sequence_t;

namespace {
   void
   inject_module_labels(intermediate_table &int_table,
                        std::string const &table_spec,
                        sequence_t &all_modules) {
      if (!int_table.exists(table_spec)) return;

      extended_value &top_table_val = int_table[table_spec];

      if (!top_table_val.is_a(TABLE)) return;

      table_t &table =
         boost::any_cast<table_t&>(top_table_val.value);

      for (table_t::iterator
              i = table.begin(),
              end_iter = table.end();
           i != end_iter;
           ++i) {
         if (i->first.find('_') != std::string::npos) {
            throw cet::exception("BAD_MODULE_LABEL")
               << "Module parameter set label \""
               << i->first
               << "\" is illegal: "
               << "underscores are not permitted in module names.";
         }
         extended_value ml(false, STRING, fhicl::detail::encode(i->first));

         int_table[table_spec + "." + i->first + ".module_label"] = ml;
         all_modules.push_back(ml);
      }
   }
}

void
art::IntermediateTablePostProcessor::
apply(intermediate_table &raw_config) const {

   // process_name
   if (!raw_config.exists("process_name")) {
      std::cerr << "INFO: using default process_name, \"DUMMY.\"\n";
      raw_config["process_name"] =
         extended_value(false, STRING, fhicl::detail::encode("DUMMY"));
   }
   std::string process_name;
   fhicl::detail::decode(raw_config.find("process_name").value, process_name);
   if (process_name.empty()) {
      throw cet::exception("BAD_PROCESS_NAME")
         << "Empty process_name not permitted.";
   } else if (process_name.find('_') != std::string::npos) {
      throw cet::exception("BAD_PROCESS_NAME")
         << "Underscores not permitted in process_name: illegal value \""
         << process_name
         << "\"";
   }

   // trigger_paths top-level pset
   if (raw_config.exists("physics.trigger_paths")) {
      raw_config["trigger_paths.trigger_paths"] =
         raw_config["physics.trigger_paths"];
   }

   // module_labels and all_modules

   // module_label for source specification (doesn't go into all_modules).
   if (raw_config.exists("source") &&
       raw_config["source"].is_a(TABLE)) {
      raw_config["source.module_label"] =
         extended_value(false, STRING, fhicl::detail::encode("source"));
   }

   sequence_t all_modules;

   inject_module_labels(raw_config, "outputs", all_modules);
   inject_module_labels(raw_config, "physics.producers", all_modules);
   inject_module_labels(raw_config, "physics.filters", all_modules);
   inject_module_labels(raw_config, "physics.analyzers", all_modules);
   raw_config["all_modules"] =
      extended_value(false, SEQUENCE, all_modules);

   // messagefacility configuration.
   if (!raw_config.exists("services.message")) {
     raw_config["services.message.destinations.STDOUT.categories.ArtReport.limit"] =
       extended_value(false, NUMBER, fhicl::detail::encode(100));
     raw_config["services.message.destinations.STDOUT.categories.default.limit"] =
       extended_value(false, NUMBER, fhicl::detail::encode(-1));
     raw_config["services.message.destinations.STDOUT.type"] =
         extended_value(false, STRING, fhicl::detail::encode("cout"));
     raw_config["services.message.destinations.STDOUT.threshold"] =
         extended_value(false, STRING, fhicl::detail::encode("INFO"));
   }
}
