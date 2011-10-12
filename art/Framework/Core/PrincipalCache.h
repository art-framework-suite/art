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
#include "art/Persistency/Provenance/SubRunID.h"

#include "cpp0x/memory"

#include <map>

namespace art {

  typedef SubRunID SubRunKey;

  class PrincipalCache {
  public:

    PrincipalCache();

    // use compiler-generated copy c'tor, copy assignment, and d'tor

    RunPrincipal & runPrincipal(RunNumber_t run);
    RunPrincipal const & runPrincipal(RunNumber_t run) const;
    std::shared_ptr<RunPrincipal> runPrincipalPtr(RunNumber_t run);

    // Current run (most recently read and inserted run)
    RunPrincipal & runPrincipal();
    RunPrincipal const & runPrincipal() const;
    std::shared_ptr<RunPrincipal> runPrincipalPtr();

    SubRunPrincipal & subRunPrincipal(RunNumber_t run, SubRunNumber_t subRun);
    SubRunPrincipal const & subRunPrincipal(RunNumber_t run, SubRunNumber_t subRun) const;
    std::shared_ptr<SubRunPrincipal> subRunPrincipalPtr(RunNumber_t run, SubRunNumber_t subRun);

    // Current subRun (most recently read and inserted subRun)
    SubRunPrincipal & subRunPrincipal();
    SubRunPrincipal const & subRunPrincipal() const;
    std::shared_ptr<SubRunPrincipal> subRunPrincipalPtr();

    bool insert(std::shared_ptr<RunPrincipal> rp);
    bool insert(std::shared_ptr<SubRunPrincipal> srp);

    bool noMoreRuns();
    bool noMoreSubRuns();

    RunPrincipal const & lowestRun() const;
    SubRunPrincipal const & lowestSubRun() const;

    void deleteLowestRun();
    void deleteLowestSubRun();

    void deleteRun(RunNumber_t run);
    void deleteSubRun(RunNumber_t run, SubRunNumber_t subRun);

  private:

    typedef std::map<RunNumber_t, std::shared_ptr<RunPrincipal> >::iterator RunIterator;
    typedef std::map<RunNumber_t, std::shared_ptr<RunPrincipal> >::const_iterator ConstRunIterator;
    typedef std::map<SubRunKey, std::shared_ptr<SubRunPrincipal> >::iterator SubRunIterator;
    typedef std::map<SubRunKey, std::shared_ptr<SubRunPrincipal> >::const_iterator ConstSubRunIterator;

    std::map<RunNumber_t, std::shared_ptr<RunPrincipal> > runPrincipals_;
    std::map<SubRunKey, std::shared_ptr<SubRunPrincipal> > subRunPrincipals_;

    std::shared_ptr<RunPrincipal> currentRunPrincipal_;
    std::shared_ptr<SubRunPrincipal> currentSubRunPrincipal_;
  };
}

#endif /* art_Framework_Core_PrincipalCache_h */

// Local Variables:
// mode: c++
// End:
