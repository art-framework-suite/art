
#include "art/Framework/Modules/Prescaler.h"
#include "art/ParameterSet/ParameterSet.h"

namespace edm
{
  Prescaler::Prescaler(edm::ParameterSet const& ps):
    count_(),
    n_(ps.getParameter<int>("prescaleFactor")),
    offset_(ps.getParameter<int>("prescaleOffset"))
  {
  }

  Prescaler::~Prescaler()
  {
  }

  bool Prescaler::filter(edm::Event & e,edm::EventSetup const&)
  {
    ++count_;
    return count_ % n_ == offset_ ? true : false;
  }

  void Prescaler::endJob()
  {
  }
}
