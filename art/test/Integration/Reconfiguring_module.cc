// ======================================================================
//
//
// ======================================================================

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/test/Integration/Reconfigurable.h"
#include "art/test/TestObjects/ToyProducts.h"

#include <iostream>
#include <memory>
#include <vector>

using namespace art;

//--------------------------------------------------------------------
//
// Produces a SimpleProduct product instance.
//
class Reconfiguring : public art::EDAnalyzer
{
public:
  explicit Reconfiguring( fhicl::ParameterSet const & p)
    : art::EDAnalyzer(p)
  {
  }

  void analyze( art::Event const & e ) override;

private:
  typedef std::vector<fhicl::ParameterSet> ParameterSets;
};

void Reconfiguring::analyze( art::Event const & )
{
  ServiceHandle<Reconfigurable> rc;
  int level_before = rc->get_debug_level();
  ServiceRegistry& inst = ServiceRegistry::instance();
  ParameterSets psets;
  inst.presentToken().getParameterSets(psets);

  ParameterSets::iterator cur=psets.begin(),end=psets.end();
  for(;cur!=end;++cur)
    {
      std::cerr << "service name = "
            << cur->get<std::string>("service_type","none")
                << "\n";
      cur->put<int>("debug_level",10);
    }
  inst.presentToken().putParameterSets(psets);

  int level_after = rc->get_debug_level();

  std::cerr << "before=" << level_before << " after=" << level_after << "\n";

  assert(level_before==0);
  assert(level_after==10);
}

// ----------------------------------------------------------------------

DEFINE_ART_MODULE(Reconfiguring)

// ======================================================================
