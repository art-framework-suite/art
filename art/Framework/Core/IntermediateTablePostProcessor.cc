#include "art/Framework/Core/IntermediateTablePostProcessor.h"

#include "boost/any.hpp"
#include "cetlib/canonical_string.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/extended_value.h"

#include <iostream>
#include <string>
#include <vector>

using namespace fhicl;

namespace {
   std::string itString(extended_value const &val) {
      std::string result = boost::any_cast<std::string>(val.value);
      if( result.size() >= 2 && result[0] == '\"' && result.end()[-1] == '\"' ) {
         return cet::unescape( result.substr(1, result.size()-2) );
      } else {
         return result;
      }
   }


   std::string canonicalize(std::string const &in) {
      if (in.empty()) {
         return in;
      }
      std::string result;
      if (!cet::canonical_string(in, result)) {
         throw cet::exception("CONFIG_POSTPROCESSING")
            << "INTERNAL ERROR: unable to canonicalize non-zero string "
            << in;
      }
      return result;
   }

   extended_value
   inject_module_labels(extended_value const &ev_in,
                        extended_value::sequence_t &all_modules) {
      extended_value::table_t table = (extended_value::table_t) ev_in;
      for (extended_value::table_t::iterator
              i = table.begin(),
              end_iter = table.end();
           i != end_iter;
           ++i) {
         if (i->first.find('_') != std::string::npos) {
            throw cet::exception("BAD_MODULE_LABEL")
               << "Module parameter set label \""
               << i->first
               << "\" is illegal: underscores are not permitted in module names.";
         }
         // Insert module_label into module config.
         extended_value::table_t mod_table = 
            (extended_value::table_t) i->second;
         mod_table["module_label"] =
            extended_value(false,
                           STRING,
                           canonicalize(i->first));
         // Insert revised module config back into module config list.
         table[i->first] =
            extended_value(i->second.in_prolog,
                           i->second.tag,
                           mod_table);
         // Insert module_label into module list.
         all_modules.push_back(extended_value(false,
                                              STRING,
                                              canonicalize(i->first)));
      }
      return extended_value(ev_in.in_prolog,
                            ev_in.tag,
                            table);
   }

}

void
art::IntermediateTablePostProcessor::
operator() (intermediate_table &raw_config) {

  // process_name
  try {
     std::string process_name = itString(raw_config.find("process_name"));
     if (process_name.empty()) {
        throw cet::exception("BAD_PROCESS_NAME")
           << "Empty process_name not permitted.";
     } else if (process_name.find('_') != std::string::npos) {
        throw cet::exception("BAD_PROCESS_NAME")
           << "Underscores not permitted in process_name: illegal value \""
           << process_name
           << "\"";
    }
  }
  catch (exception &e) {
     if (e.categoryCode() == cant_find) {
        std::cerr << "INFO: using default process_name, \"DUMMY.\"\n";
        raw_config.insert("process_name", false, STRING, canonicalize("DUMMY"));
     } else {
        throw;
     }
  }

  // trigger_paths top-level pset
  {
     try {
        extended_value physics = raw_config.find("physics");
        extended_value::table_t physics_table =
           (extended_value::table_t) physics;
        extended_value::table_t::const_iterator it =
           physics_table.find("trigger_paths");
        if (it != physics_table.end()) {
           extended_value::table_t tp_table;
           tp_table["trigger_paths"] = it->second;
           raw_config.insert("trigger_paths", false, TABLE, tp_table);
        }
     }
     catch (exception &e) {
        if (e.categoryCode() == cant_find) {
           // Ignore
        } else {
           throw;
        }
     }
  }

  // module_labels and all_modules
  {
     // Note that we can't edit-in-place, so we have to copy and
     // re-insert, taking care to follow the hierarchy correctly.
     extended_value::sequence_t all_modules;
     try {
        extended_value outputs_new =
           inject_module_labels(raw_config.find("outputs"), all_modules);
        raw_config.insert("outputs", outputs_new);
     }
     catch (exception &e) {
        if (e.categoryCode() == cant_find) {
           // Ignore
        } else {
           throw;
        }
     }
     try {
        extended_value physics = raw_config.find("physics");
        extended_value::table_t physics_table =
           (extended_value::table_t) physics;
        std::vector<std::string> physics_psets;
        physics_psets.push_back("producers");
        physics_psets.push_back("filters");
        physics_psets.push_back("analyzers");
        for (std::vector<std::string>::const_iterator
                i = physics_psets.begin(),
                end_iter = physics_psets.end();
             i != end_iter;
             ++i) {
           try {
              physics_table[*i] =
                 inject_module_labels(physics_table[*i],
                                      all_modules);
           }
           catch (exception &e) {
              if (e.categoryCode() == cant_find) {
                 // Ignore
              } else {
                 throw;
              }
           }
        }
        raw_config.insert("physics", 
                          physics.in_prolog,
                          physics.tag,
                          physics_table);
     }
     catch (exception &e) {
        if (e.categoryCode() == cant_find) {
           // Ignore
        } else {
           throw;
        }
     }
     raw_config.insert("all_modules", false, SEQUENCE, all_modules);
  }

  // module_label for source specification.
   try {
      extended_value const & sources_old =
         raw_config.find("source");
      extended_value::table_t source_table =
         (extended_value::table_t) sources_old;
      source_table["module_label"] =
         extended_value(false,
                        STRING,
                        canonicalize("source"));
      raw_config.insert("source",
                        sources_old.in_prolog,
                        sources_old.tag,
                        source_table);
   }
   catch (exception &e) {
      if (e.categoryCode() == cant_find) {
         // Ignore
      } else {
         throw;
      }
   }
}
