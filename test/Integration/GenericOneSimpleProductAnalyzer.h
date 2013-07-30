#ifndef test_Integration_GenericOneSimpleProductAnalyzer_h
#define test_Integration_GenericOneSimpleProductAnalyzer_h

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Utilities/detail/metaprogramming.h"
#include "cetlib/exception.h"
#include "cpp0x/type_traits"
#include "fhiclcpp/ParameterSet.h"
#include <string>

namespace arttest {
  template <typename V, typename P> class GenericOneSimpleProductAnalyzer;
  namespace detail {
    // All this detail is to decide whether our product P has a "value"
    // member or is (effectively) converitible-to-V.
    template <typename V, typename P, V P::*> struct value_member;
    template <typename V, typename P> art::detail::no_tag  has_value_helper(...);
    template <typename V, typename P> art::detail::yes_tag has_value_helper(value_member<V, P, &P::value> *x);

    template <typename V, typename P>
    struct has_value_member
    {
      static bool const value =
        sizeof(has_value_helper<V, P>(0)) == sizeof(art::detail::yes_tag);
    };

    template <typename V, typename P>
    struct GetValue
    {
      V const & operator()(art::Handle<P> const &h) { return h->value; }
    };

    template <typename V, typename P>
    struct DereferenceHandle
    {
      V const & operator()(art::Handle<P> const &h) { return *h; }
    };

  }
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
    if (require_presence_) {
      typename std::conditional<detail::has_value_member<V, P>::value, detail::GetValue<V, P>, detail::DereferenceHandle<V, P> >::type get_value;
      if (get_value(handle) != value_) {
        throw cet::exception("ValueMismatch")
          << "The value for \"" << input_label_
          << "\" is " << get_value(handle)
          << " but was supposed to be " << value_
          << '\n';
      }
    }
  }

  void reconfigure(fhicl::ParameterSet const & pset) override {
    input_label_ = pset.get<std::string>("input_label");
    require_presence_ = pset.get<bool>("require_presence", true);
    if (require_presence_) {
      value_ = pset.get<V>("expected_value");
    } else {
      V tmp;
      std::swap(value_, tmp);
    }
  }

private:
  V value_;
  std::string input_label_;
  bool require_presence_;
};

#endif /* test_Integration_GenericOneSimpleProductAnalyzer_h */

// Local Variables:
// mode: c++
// End:
