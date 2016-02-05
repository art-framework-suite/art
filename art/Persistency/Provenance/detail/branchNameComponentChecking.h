#ifndef art_Persistency_Provenance_detail_branchNameComponentChecking_h
#define art_Persistency_Provenance_detail_branchNameComponentChecking_h

#include "canvas/Persistency/Provenance/BranchKey.h"

#include <string>

namespace art {
  namespace detail {
    BranchKey splitToComponents(std::string const & branchName,
                                std::string & errMsg);
    bool checkBranchNameSelector(std::string const & branchNameSelector,
                                 std::string & errMsg);
    bool checkBranchNameSelector(BranchKey const & components,
                                 std::string & errMsg);
    bool checkFriendlyName(std::string const & friendlyName,
                           std::string & errMsg);
    bool checkFriendlyNameSelector(std::string const & friendlyNameSelector,
                                   std::string & errMsg);
    bool checkModuleLabel(std::string const & moduleLabel,
                          std::string & errMsg);
    bool checkModuleLabelSelector(std::string const & moduleLabelSelector,
                                  std::string & errMsg);
    bool checkInstanceName(std::string const & instanceName,
                           std::string & errMsg);
    bool checkInstanceNameSelector(std::string const & instanceNameSelector,
                                   std::string & errMsg);
    bool checkProcessName(std::string const & processName,
                          std::string & errMsg);
    bool checkProcessNameSelector(std::string const & processNameSelector,
                                  std::string & errMsg);
  }
}

#endif /* art_Persistency_Provenance_detail_branchNameComponentChecking_h */

// Local Variables:
// mode: c++
// End:
