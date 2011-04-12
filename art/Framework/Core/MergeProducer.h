#ifndef art_Framework_Core_MergeProducer_h
#define art_Framework_Core_MergeProducer_h

#include "art/Framework/Core/EDProducer.h"

namespace art  {
  template <class T>
  class MergeProducer;
}

template <class T>
class art::MergeProducer : public art::EDProducer {
public:
  typedef MergeDetail
  explicit MergeProducer(fhicl::ParameterSet const &p);
  virtual ~MergeProducer();

  virtual void produce(art::Event &e);

private:
  ProductMergeHelper helper_;
  T detail_;
  std::auto_ptr<RootInput> source_;
};

template <class T>
art::MergeProducer::MergeProducer(fhicl::ParameterSet const &p)
  :
  EDProducer(),
  helper_(),
  detail_(p, helper),
  source_(new RootInput(p, detail::))
{
}

template <class T>
art::MergeProducer::produce(art::Event &e) {

}
#endif /* art_Framework_Core_MergeProducer_h */

// Local Variables:
// mode: c++
// End:
