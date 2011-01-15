#include "NovaConfigPostProcessor.h"

#include "boost/any.hpp"
#include "cetlib/canonical_string.h"
#include "cetlib/exception.h"
#include "fhiclcpp/exception.h"
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
}

NovaConfigPostProcessor::NovaConfigPostProcessor()
   :
   source_(),
   tFileName_(),
   output_(),
   nevts_(),
   startEvt_(),
   skipEvts_(),
   wantNevts_(false),
   wantStartEvt_(false),
   wantSkipEvts_(false)
{
}

void NovaConfigPostProcessor::apply(intermediate_table  &raw_config) const {
   applySource(raw_config);
   applyOutput(raw_config);
   applyTFileName(raw_config);
}
