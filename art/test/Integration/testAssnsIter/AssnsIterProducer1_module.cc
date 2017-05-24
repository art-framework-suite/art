////////////////////////////////////////////////////////////////////////
// Class:       AssnsIterProducer1
// Plugin Type: producer (art v2_05_00)
// File:        AssnsIterProducer1_module.cc
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


namespace lartest {
   class AssnsIterProducer1;
}

using lartest::AssnsIterProducer1;

class lartest::AssnsIterProducer1 : public art::EDProducer {
public:
   typedef  std::vector<int>             intvec_t;
   typedef  std::vector<std::string>     strvec_t;
   typedef  std::vector<float>           floatvec_t;
   
   explicit AssnsIterProducer1(fhicl::ParameterSet const & p);
   // The compiler-generated destructor is fine for non-base
   // classes without bare pointers or other resource use.
   
   // Plugins should not be copied or assigned.
   AssnsIterProducer1(AssnsIterProducer1 const &) = delete;
   AssnsIterProducer1(AssnsIterProducer1 &&) = delete;
   AssnsIterProducer1 & operator = (AssnsIterProducer1 const &) = delete;
   AssnsIterProducer1 & operator = (AssnsIterProducer1 &&) = delete;
   
   // Required functions.
   void produce(art::Event & e) override;
   
private:
std::string fInputLabel;
};


AssnsIterProducer1::AssnsIterProducer1(fhicl::ParameterSet const & p)
 :  fInputLabel(p.get<std::string>("input_label"))
{
   produces<intvec_t>();
   produces<strvec_t>();
   produces<floatvec_t>();
}

void AssnsIterProducer1::produce(art::Event & e)
{
   auto vs = std::make_unique<strvec_t>(strvec_t {"one", "one-a", "two", "two-a", "three", "three-a"});
   auto vi = std::make_unique<intvec_t>(intvec_t {1, 2, 3});
   auto vf = std::make_unique<floatvec_t>(floatvec_t {1.0, 1.1, 2.0, 2.1, 3.0, 3.1 });
   
   e.put(std::move(vs));
   e.put(std::move(vi));
   e.put(std::move(vf));
}

DEFINE_ART_MODULE(AssnsIterProducer1)
