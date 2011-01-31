// ======================================================================
//
// checkObsoleteParameters
//
// ======================================================================

#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include <algorithm>
#include <string>
#include <vector>

// ----------------------------------------------------------------------

std::string
  checkObsoleteParameters( fhicl::ParameterSet const & pset
                         , std::string         const & new_name
                         , std::string         const & old_name
                         )
{
  std::vector<std::string> names = pset.get_keys();
  bool has_new = std::find(names.begin(), names.end(), new_name) != names.end();
  bool has_old = std::find(names.begin(), names.end(), old_name) != names.end();

  if( has_new && has_old )
    mf::LogWarning("OUTDATED")
      << "ParameterSet contains entries for deprecated name " << old_name
      << " as well as for preferred name " << new_name
      << ";\ndeprecated name ignored.";

  else if( has_old ) {
    mf::LogWarning("OUTDATED")
      << "ParameterSet contains entry for deprecated name " << old_name
      << " instead of for preferred name " << new_name
      << ";\ndeprecated name used in this run, but should be updated.";
    return old_name;
  }

  else
    return new_name;
}

// ======================================================================
