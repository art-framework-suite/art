#ifndef test_Integration_oldtest_HierarchicalAlgorithms_h
#define test_Integration_oldtest_HierarchicalAlgorithms_h

/** \class alg_1 and alg_2
 *
 ************************************************************/

#include <vector>
#include "art/ParameterSet/ParameterSet.h"

namespace arttest {

  class alg_2
  {
  public:
    explicit alg_2(const art::ParameterSet& ps) :
      flavor_(ps.getParameter<std::string>("flavor")),
      debugLevel_(ps.getUntrackedParameter<int>("debug", 0))
    { }

    std::string& flavor() { return flavor_; }

  private:
    std::string flavor_;
    int         debugLevel_;
  };

  class alg_1
  {
  public:
    explicit alg_1(const art::ParameterSet& ps) :
      count_(ps.getParameter<int>("count")),
      inner_alg_(ps.getParameter<art::ParameterSet>("nest_2"))
    { }

  private:
    int   count_;
    alg_2 inner_alg_;
  };

}

#endif /* test_Integration_oldtest_HierarchicalAlgorithms_h */

// Local Variables:
// mode: c++
// End:
