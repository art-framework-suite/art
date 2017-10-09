// ======================================================================
//
// Produces an PtrVector instance.
//
// ======================================================================

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Common/PtrVector.h"
#include "fhiclcpp/ParameterSet.h"

#include <iostream>
#include <memory>
#include <vector>

namespace arttest {
  class PtrVectorProducer;
}

using arttest::PtrVectorProducer;

// ----------------------------------------------------------------------

class arttest::PtrVectorProducer : public art::EDProducer {
public:
  typedef std::vector<int> intvector_t;
  typedef art::PtrVector<int> product_t;

  explicit PtrVectorProducer(fhicl::ParameterSet const& p)
    : input_label_(p.get<std::string>("input_label"))
  {
    produces<product_t>();
  }

  void produce(art::Event& e) override;

private:
  std::string input_label_;

}; // PtrVectorProducer

// ----------------------------------------------------------------------

void
PtrVectorProducer::produce(art::Event& e)
{
  std::cerr << "PtrVectorProducer::produce is running!\n";

  art::Handle<intvector_t> h;
  e.getByLabel(input_label_, h);

  std::unique_ptr<product_t> prod(new product_t);
  for (int k = 0; k != 8; ++k) {
    art::Ptr<int> p(h, 7 - k);
    prod->push_back(p);
  }
  prod->sort();

  e.put(std::move(prod));
}

// ----------------------------------------------------------------------

DEFINE_ART_MODULE(PtrVectorProducer)

// ======================================================================
