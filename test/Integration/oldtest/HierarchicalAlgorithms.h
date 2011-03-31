#ifndef test_Integration_oldtest_HierarchicalAlgorithms_h
#define test_Integration_oldtest_HierarchicalAlgorithms_h

/** \class alg_1 and alg_2
 *
 ************************************************************/

#include <vector>
#include "fhiclcpp/ParameterSet.h"

namespace arttest {

  class alg_2
  {
  public:
    explicit alg_2(const fhicl::ParameterSet& ps) :
      flavor_(ps.get<std::string>("flavor")),
      debugLevel_(ps.get<int>("debug", 0))
    { }

    std::string& flavor() { return flavor_; }

  private:
    std::string flavor_;
    int         debugLevel_;
  };

  class alg_1
  {
  public:
    explicit alg_1(const fhicl::ParameterSet& ps) :
      count_(ps.get<int>("count")),
      inner_alg_(ps.get<fhicl::ParameterSet>("nest_2"))
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
