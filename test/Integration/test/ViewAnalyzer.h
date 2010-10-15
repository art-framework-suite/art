#ifndef Integration_ViewAnalyzer_h
#define Integration_ViewAnalyzer_h

#include <string>

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDAnalyzer.h"

namespace edmtest
{

  class ViewAnalyzer : public art::EDAnalyzer
  {
  public:
    explicit ViewAnalyzer(art::ParameterSet const& /* no parameters*/);
    virtual ~ViewAnalyzer();
    virtual void analyze(art::Event const& e,
			 art::EventSetup const& /* unused */ );

    template <class P, class V>
    void testProduct(art::Event const& e,
		     std::string const& moduleLabel) const;

    void testDSVProduct(art::Event const& e,
			std::string const& moduleLabel) const;

    void testProductWithBaseClass(art::Event const& e,
 			          std::string const& moduleLabel) const;

    void testRefVector(art::Event const& e,
		       std::string const& moduleLabel) const;

    void testRefToBaseVector(art::Event const& e,
			     std::string const& moduleLabel) const;
  };

}

#endif
