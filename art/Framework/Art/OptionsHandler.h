#ifndef art_Framework_Art_OptionsHandler_h
#define art_Framework_Art_OptionsHandler_h

// Simple interface to allow easy addition of experiment-specific
// command-line options and their processing.

#include "fhiclcpp/fwd.h"

#include "boost/program_options.hpp"
namespace bpo = boost::program_options;

namespace art {
  class OptionsHandler;
}

class art::OptionsHandler {
public:
  virtual ~OptionsHandler() = default;
  int checkOptions(bpo::variables_map const & vm);
  int processOptions(bpo::variables_map const & vm,
                     fhicl::intermediate_table & raw_config);
private:
  // Check selected options for consistency (should throw on failure).
  virtual int doCheckOptions(bpo::variables_map const & vm) = 0;
  // Act on selected options  (should throw on failure).
  virtual int doProcessOptions(bpo::variables_map const & vm,
                               fhicl::intermediate_table & raw_config) = 0;
};
#endif /* art_Framework_Art_OptionsHandler_h */

// Local Variables:
// mode: c++
// End:
