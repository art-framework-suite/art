#ifndef art_test_Integration_GenericOneSimpleProductAnalyzer_h
#define art_test_Integration_GenericOneSimpleProductAnalyzer_h

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "cetlib/metaprogramming.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"

#include <string>
#include <type_traits>

namespace arttest {
  template <typename V, typename P>
  class GenericOneSimpleProductAnalyzer;
  namespace detail {

    // All this detail is to decide whether our product P has a "value"
    // member or is (effectively) convertible-to-V.

    template <typename V, typename P, V P::*>
    struct value_member;
    template <typename V, typename P>
    cet::no_tag has_value_helper(...);
    template <typename V, typename P>
    cet::yes_tag has_value_helper(value_member<V, P, &P::value>* x);

    template <typename V, typename P>
    struct has_value_member {
      static bool constexpr value{sizeof(has_value_helper<V, P>(nullptr)) ==
                                  sizeof(cet::yes_tag)};
    };

    template <typename V, typename P>
    struct GetValue {
      V const&
      operator()(art::Handle<P> const& h)
      {
        return h->value;
      }
    };

    template <typename V, typename P>
    struct DereferenceHandle {
      V const&
      operator()(art::Handle<P> const& h)
      {
        return *h;
      }
    };

  } // namespace detail
} // namespace arttest

template <typename V, typename P>
class arttest::GenericOneSimpleProductAnalyzer : public art::EDAnalyzer {
public:
  struct Config {

    Config()
      : input_label{fhicl::Name{"input_label"}}
      , branch_type{fhicl::Name{"branch_type"}, art::InEvent}
      , require_presence{fhicl::Name{"require_presence"},
                         fhicl::Comment{
                           "If 'require_presence' is true, then the product\n"
                           "must be successfully retrievable.  If false, then\n"
                           "the product must NOT be successfully "
                           "retrievable--i.e.\n"
                           "a call to 'getByLabel' must return false."},
                         true}
      , expected_value{
          fhicl::Name{"expected_value"},
          fhicl::Comment{"The value of the following parameter is retrieved\n"
                         "only if require_presence is 'true'."},
          [this] { return require_presence(); }}
    {}

    fhicl::Atom<std::string> input_label;
    fhicl::Atom<unsigned> branch_type;
    fhicl::Atom<bool> require_presence;
    fhicl::Atom<V> expected_value;
  };

  using Parameters = EDAnalyzer::Table<Config>;
  GenericOneSimpleProductAnalyzer(Parameters const& ps)
    : EDAnalyzer{ps}
    , input_label_{ps().input_label()}
    , branch_type_{art::BranchType(ps().branch_type())}
    , require_presence_{ps().require_presence()}
  {
    switch (branch_type_) {
      case art::InEvent:
        consumes<P>(input_label_);
        break;
      case art::InSubRun:
        consumes<P, art::InSubRun>(input_label_);
        break;
      case art::InRun:
        consumes<P, art::InRun>(input_label_);
        break;
      default:
        throw cet::exception("BranchTypeMismatch")
          << "Branch type: " << branch_type_
          << "not supported for this module.";
    }
    if (require_presence_) {
      value_ = ps().expected_value();
    }
  }

  void
  verify_value(art::BranchType const bt, art::Handle<P> const& h) const
  {
    std::conditional_t<detail::has_value_member<V, P>::value,
                       detail::GetValue<V, P>,
                       detail::DereferenceHandle<V, P>>
      get_value;
    if (get_value(h) != value_) {
      throw cet::exception("ValueMismatch")
        << "The value for \"" << input_label_ << "\", branchType \"" << bt
        << "\" is " << get_value(h) << " but was supposed to be " << value_
        << '\n';
    }
  }

  void
  analyze(art::Event const& e) override
  {
    if (branch_type_ != art::InEvent)
      return;
    art::Handle<P> handle;
    e.getByLabel(input_label_, handle);
    assert(handle.isValid() == require_presence_);
    if (require_presence_) {
      verify_value(art::InEvent, handle);
    }
  }

  void
  endSubRun(art::SubRun const& sr) override
  {
    if (branch_type_ != art::InSubRun)
      return;
    art::Handle<P> handle;
    sr.getByLabel(input_label_, handle);
    assert(handle.isValid() == require_presence_);
    if (require_presence_) {
      verify_value(art::InSubRun, handle);
    }
  }

  void
  endRun(art::Run const& r) override
  {
    if (branch_type_ != art::InRun)
      return;
    art::Handle<P> handle;
    r.getByLabel(input_label_, handle);
    assert(handle.isValid() == require_presence_);
    if (require_presence_) {
      verify_value(art::InRun, handle);
    }
  }

private:
  V value_{};
  std::string input_label_;
  art::BranchType branch_type_;
  bool require_presence_;
};

#endif /* art_test_Integration_GenericOneSimpleProductAnalyzer_h */

// Local Variables:
// mode: c++
// End:
