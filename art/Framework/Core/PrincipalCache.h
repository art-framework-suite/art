#ifndef FWCore_Framework_PrincipalCache_h
#define FWCore_Framework_PrincipalCache_h

/*


Designed to save RunPrincipal's and SubRunPrincipal's
in memory.  Manages merging of products in those principals
when there is more than one principal from the same run
or subRun.

Original Author: W. David Dagenhart
*/

#include "art/Framework/Core/Frameworkfwd.h"

#include "boost/shared_ptr.hpp"

#include <map>

namespace art {

  class SubRunKey {
  public:
    int run() { return run_; }
    int subRun() { return subRun_; }

    SubRunKey(int run, int subRun) : run_(run), subRun_(subRun) { }

    bool operator<(const SubRunKey& right) const {
      if (run_ == right.run_) return subRun_ < right.subRun_;
      return run_ < right.run_;
    }

  private:
    int run_;
    int subRun_;
  };

  class PrincipalCache
  {
  public:

    PrincipalCache();
    ~PrincipalCache();

    RunPrincipal & runPrincipal(int run);
    RunPrincipal const& runPrincipal(int run) const;
    boost::shared_ptr<RunPrincipal> runPrincipalPtr(int run);

    // Current run (most recently read and inserted run)
    RunPrincipal & runPrincipal();
    RunPrincipal const& runPrincipal() const;
    boost::shared_ptr<RunPrincipal> runPrincipalPtr();

    SubRunPrincipal & subRunPrincipal(int run, int subRun);
    SubRunPrincipal const& subRunPrincipal(int run, int subRun) const;
    boost::shared_ptr<SubRunPrincipal> subRunPrincipalPtr(int run, int subRun);

    // Current subRun (most recently read and inserted subRun)
    SubRunPrincipal & subRunPrincipal();
    SubRunPrincipal const& subRunPrincipal() const;
    boost::shared_ptr<SubRunPrincipal> subRunPrincipalPtr();

    bool insert(boost::shared_ptr<RunPrincipal> rp);
    bool insert(boost::shared_ptr<SubRunPrincipal> lbp);

    bool noMoreRuns();
    bool noMoreSubRuns();

    RunPrincipal const& lowestRun() const;
    SubRunPrincipal const& lowestSubRun() const;

    void deleteLowestRun();
    void deleteLowestSubRun();

    void deleteRun(int run);
    void deleteSubRun(int run, int subRun);

  private:

    typedef std::map<int, boost::shared_ptr<RunPrincipal> >::iterator RunIterator;
    typedef std::map<int, boost::shared_ptr<RunPrincipal> >::const_iterator ConstRunIterator;
    typedef std::map<SubRunKey, boost::shared_ptr<SubRunPrincipal> >::iterator SubRunIterator;
    typedef std::map<SubRunKey, boost::shared_ptr<SubRunPrincipal> >::const_iterator ConstSubRunIterator;

    std::map<int, boost::shared_ptr<RunPrincipal> > runPrincipals_;
    std::map<SubRunKey, boost::shared_ptr<SubRunPrincipal> > subRunPrincipals_;

    boost::shared_ptr<RunPrincipal> currentRunPrincipal_;
    boost::shared_ptr<SubRunPrincipal> currentSubRunPrincipal_;
  };
}

#endif
