#ifndef FWCore_Framework_PrincipalCache_h
#define FWCore_Framework_PrincipalCache_h

/*


Designed to save RunPrincipal's and SubRunPrincipal's
in memory.  Manages merging of products in those principals
when there is more than one principal from the same run
or luminosity block.

Original Author: W. David Dagenhart
*/

#include "art/Framework/Core/Frameworkfwd.h"

#include "boost/shared_ptr.hpp"

#include <map>

namespace edm {

  class SubRunKey {
  public:
    int run() { return run_; }
    int lumi() { return lumi_; }

    SubRunKey(int run, int lumi) : run_(run), lumi_(lumi) { }

    bool operator<(const SubRunKey& right) const {
      if (run_ == right.run_) return lumi_ < right.lumi_;
      return run_ < right.run_;
    }

  private:
    int run_;
    int lumi_;
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

    SubRunPrincipal & lumiPrincipal(int run, int lumi);
    SubRunPrincipal const& lumiPrincipal(int run, int lumi) const;
    boost::shared_ptr<SubRunPrincipal> lumiPrincipalPtr(int run, int lumi);

    // Current luminosity block (most recently read and inserted luminosity block)
    SubRunPrincipal & lumiPrincipal();
    SubRunPrincipal const& lumiPrincipal() const;
    boost::shared_ptr<SubRunPrincipal> lumiPrincipalPtr();

    bool insert(boost::shared_ptr<RunPrincipal> rp);
    bool insert(boost::shared_ptr<SubRunPrincipal> lbp);

    bool noMoreRuns();
    bool noMoreLumis();

    RunPrincipal const& lowestRun() const;
    SubRunPrincipal const& lowestLumi() const;

    void deleteLowestRun();
    void deleteLowestLumi();

    void deleteRun(int run);
    void deleteLumi(int run, int lumi);

  private:

    typedef std::map<int, boost::shared_ptr<RunPrincipal> >::iterator RunIterator;
    typedef std::map<int, boost::shared_ptr<RunPrincipal> >::const_iterator ConstRunIterator;
    typedef std::map<SubRunKey, boost::shared_ptr<SubRunPrincipal> >::iterator SubRunIterator;
    typedef std::map<SubRunKey, boost::shared_ptr<SubRunPrincipal> >::const_iterator ConstSubRunIterator;

    std::map<int, boost::shared_ptr<RunPrincipal> > runPrincipals_;
    std::map<SubRunKey, boost::shared_ptr<SubRunPrincipal> > lumiPrincipals_;

    boost::shared_ptr<RunPrincipal> currentRunPrincipal_;
    boost::shared_ptr<SubRunPrincipal> currentLumiPrincipal_;
  };
}

#endif
