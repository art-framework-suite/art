#ifndef art_Framework_Core_PrincipalCache_h
#define art_Framework_Core_PrincipalCache_h

/*


Designed to save RunPrincipal's and SubRunPrincipal's
in memory.  Manages merging of products in those principals
when there is more than one principal from the same run
or subRun.

Original Author: W. David Dagenhart
*/

#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "canvas/Persistency/Provenance/SubRunID.h"

#include <map>
#include <memory>

namespace art {

   typedef SubRunID SubRunKey;

  class PrincipalCache
  {
  public:

    PrincipalCache();

    // use compiler-generated copy c'tor, copy assignment, and d'tor

    RunPrincipal & runPrincipal(RunID run);
    RunPrincipal const& runPrincipal(RunID run) const;
    std::shared_ptr<RunPrincipal> runPrincipalPtr(RunID run);

    // Current run (most recently read and inserted run)
    RunPrincipal & runPrincipal();
    RunPrincipal const& runPrincipal() const;
    std::shared_ptr<RunPrincipal> runPrincipalPtr();

    SubRunPrincipal & subRunPrincipal(SubRunID const & sr);
    SubRunPrincipal const& subRunPrincipal(SubRunID const & sr) const;
    std::shared_ptr<SubRunPrincipal> subRunPrincipalPtr(SubRunID const & sr);

    // Current subRun (most recently read and inserted subRun)
    SubRunPrincipal & subRunPrincipal();
    SubRunPrincipal const& subRunPrincipal() const;
    std::shared_ptr<SubRunPrincipal> subRunPrincipalPtr();

    bool insert(std::shared_ptr<RunPrincipal> rp);
    bool insert(std::shared_ptr<SubRunPrincipal> srp);

    bool noMoreRuns() const;
    bool noMoreSubRuns() const;

    RunPrincipal & lowestRun() const;
    SubRunPrincipal & lowestSubRun() const;

    void deleteRun(RunID run);
    void deleteSubRun(SubRunID const & sr);

    void deleteAllPrincipals();

  private:

    std::map<RunID, std::shared_ptr<RunPrincipal> > runPrincipals_;
    std::map<SubRunKey, std::shared_ptr<SubRunPrincipal> > subRunPrincipals_;

    std::shared_ptr<RunPrincipal> currentRunPrincipal_;
    std::shared_ptr<SubRunPrincipal> currentSubRunPrincipal_;
  };
}

#endif /* art_Framework_Core_PrincipalCache_h */

// Local Variables:
// mode: c++
// End:
