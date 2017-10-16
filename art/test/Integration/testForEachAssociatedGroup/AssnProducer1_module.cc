////////////////////////////////////////////////////////////////////////
// Class:       AssnProducer1
// Plugin Type: producer (art v2_05_00)
// File:        AssnProducer1_module.cc
//
// Generated at Thu Dec  8 12:20:56 2016 by Saba Sehrish using cetskelgen
// from cetlib version v1_21_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"


namespace arttest {
   class AssnProducer1;
}

using arttest::AssnProducer1;

class arttest::AssnProducer1 : public art::EDProducer {
public:
   typedef  std::vector<int>             intvec_t;
   typedef  std::vector<std::string>     strvec_t;
   
   explicit AssnProducer1(fhicl::ParameterSet const & p);
   // The compiler-generated destructor is fine for non-base
   // classes without bare pointers or other resource use.
   
   // Plugins should not be copied or assigned.
   AssnProducer1(AssnProducer1 const &) = delete;
   AssnProducer1(AssnProducer1 &&) = delete;
   AssnProducer1 & operator = (AssnProducer1 const &) = delete;
   AssnProducer1 & operator = (AssnProducer1 &&) = delete;
   
   // Required functions.
   void produce(art::Event & e) override;
   
private:
  
};


AssnProducer1::AssnProducer1(fhicl::ParameterSet const & p)
{
   produces<intvec_t>();
   produces<strvec_t>();
}

void AssnProducer1::produce(art::Event & e)
{
   auto vs = std::make_unique<strvec_t>(strvec_t {"one", "one-a", "two", "two-a", "three", "three-a"});
   auto vi = std::make_unique<intvec_t>(intvec_t {1, 2, 3});
  
   e.put(std::move(vs));
   e.put(std::move(vi));
}

DEFINE_ART_MODULE(AssnProducer1)
