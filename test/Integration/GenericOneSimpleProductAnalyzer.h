#ifndef test_Integration_GenericOneSimpleProducAnalyzer_h
#define test_Integration_GenericOneSimpleProducAnalyzer_h

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/Event.h"
#include "art/Persistency/Common/Handle.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace arttest {
   template <typename V, typename P> class GenericOneSimpleProductAnalyzer;
}

template <typename V, typename P> class arttest::GenericOneSimpleProductAnalyzer
   : public art::EDAnalyzer
{
 public:
   GenericOneSimpleProductAnalyzer(fhicl::ParameterSet const &conf) :
      value_(),
      input_label_(conf.get<std::string>("input_label")),
      require_presence_(conf.get<bool>("require_presence", true))
         {
            if (require_presence_) {
               value_ = conf.get<V>("expected_value");
            }
         }

      void analyze(const art::Event &e) {
         art::Handle<P> handle;
         e.getByLabel(input_label_, handle);
         assert (handle.isValid() == require_presence_);
         if (require_presence_)
            if(handle->value != value_) {
               throw cet::exception("ValueMismatch")
                  << "The value for \"" << input_label_
                  << "\" is " << handle->value
                  << " but was supposed to be " << value_
                  << '\n';
            }
      }

 private:
   V value_;
   std::string input_label_;
   bool require_presence_;
};

#endif
