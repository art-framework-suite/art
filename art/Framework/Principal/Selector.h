#ifndef art_Framework_Principal_Selector_h
#define art_Framework_Principal_Selector_h
// vim: set sw=2 expandtab :

// =====================================================================
// Classes for all "selector" objects, used to select EDProducts based
// on information in the associated Provenance.
//
// Users can use the classes defined below
//
//   InputTagListSelector
//   ModuleLabelSelector
//   ProcessNameSelector
//   ProductInstanceNameSelector
//   SelectorByFunction
//
// Users can also use the class Selector, which can be constructed
// given a logical expression formed from any other selectors,
// combined with && (the AND operator), || (the OR operator) or ! (the
// NOT operator).
//
// For example, to select only products produced by a module with
// label "mymodule" and made in the process "PROD", one can use:
//
//   Selector s{ModuleLabelSelector("mymodule") &&
//              ProcessNameSelector("PROD")};
//
// If a module (EDProducter, EDFilter, EDAnalyzer, or OutputModule) is
// to use such a selector, it is best to initialize it directly upon
// construction of the module, rather than creating a new Selector
// instance for every event.
// =====================================================================

#include "art/Framework/Principal/SelectorBase.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"

#include <functional>
#include <memory>
#include <string>
#include <type_traits>

namespace art {
  template <typename T>
  concept is_selector = std::derived_from<std::remove_reference_t<T>, SelectorBase>;
  // constexpr bool is_selector =
  //   std::is_base_of_v<SelectorBase, std::remove_reference_t<T>>;

  //--------------------------------------------------------------------
  // Class ProcessNameSelector.
  // Selects EDProducts based upon process name.
  //
  // As a special case, a ProcessNameSelector created with the string
  // "*" matches *any* process (and so is rather like having no
  // ProcessNameSelector at all).  The ProcessNameSelector does *not*
  // understand the string "current_process" as a match to the current
  // process name.  To do so with the current design would require the
  // use of accessing global data, which we would like to avoid.  If
  // such matching is desired in the future, a redesign of the selector
  // system could be considered.  For now, if users wish to retrieve
  // products with the process name "current_process", they must use the
  // getBy* facilities provided by Event and friends.
  // -------------------------------------------------------------------

  class ProcessNameSelector : public SelectorBase {
  public:
    explicit ProcessNameSelector(std::string const& pn)
      : pn_{pn.empty() ? std::string{"*"} : pn}
    {}

  private:
    bool
    doMatch(BranchDescription const& p) const override
    {
      return (pn_ == "*") || (p.processName() == pn_);
    }

    std::string
    doPrint(std::string const& indent) const override
    {
      std::string result{indent + "Process name: "};
      if (pn_ == "*") {
        result += "(empty)";
      } else {
        result += "'" + pn_ + "'";
      }
      return result;
    }

    std::string pn_;
  };

  //------------------------------------------------------------------
  // Class ProductInstanceNameSelector.
  // Selects EDProducts based upon product instance name.
  //------------------------------------------------------------------

  class ProductInstanceNameSelector : public SelectorBase {
  public:
    explicit ProductInstanceNameSelector(std::string const& pin) : pin_{pin} {}

  private:
    bool
    doMatch(BranchDescription const& p) const override
    {
      return p.productInstanceName() == pin_;
    }

    std::string
    doPrint(std::string const& indent) const override
    {
      return indent + "Product instance name: '" + pin_ + '\'';
    }

    std::string pin_;
  };

  //------------------------------------------------------------------
  // Class ModuleLabelSelector.
  // Selects EDProducts based upon module label.
  //------------------------------------------------------------------

  class ModuleLabelSelector : public SelectorBase {
  public:
    explicit ModuleLabelSelector(std::string const& label) : label_{label} {}

  private:
    bool
    doMatch(BranchDescription const& p) const override
    {
      return p.moduleLabel() == label_;
    }

    std::string
    doPrint(std::string const& indent) const override
    {
      return indent + "Module label: '" + label_ + '\'';
    }

    std::string label_;
  };

  //------------------------------------------------------------------
  // Class MatchAllSelector.
  // Dummy selector whose match function always returns true.
  //------------------------------------------------------------------

  class MatchAllSelector : public SelectorBase {
    bool
    doMatch(BranchDescription const&) const override
    {
      return true;
    }

    std::string
    doPrint(std::string const&) const override
    {
      return {};
    }
  };

  // Select products based on the result of a filter function (or
  // functor) provided to the constructor.
  class SelectorByFunction : public art::SelectorBase {
  public:
    template <typename FUNC>
    explicit SelectorByFunction(FUNC func, std::string description)
      : func_(func), description_(description)
    {}

  private:
    bool
    doMatch(art::BranchDescription const& p) const override
    {
      return func_(p);
    }

    std::string
    doPrint(std::string const& indent) const override
    {
      return indent + description_;
    }

    std::function<bool(art::BranchDescription const&)> func_;
    std::string description_;
  };

  // Select products based on a sequence of wanted input tags.
  class InputTagListSelector : public art::SelectorBase {
  public:
    // Initialize wanted input tags from a sequence denoted by
    // [begin, end).
    //
    // This constructor is only valid (via SFINAE) if the provided
    // iterators dereference to a type convertible to art::InputTag.
    template <typename IT>
    InputTagListSelector(
      IT begin,
      IT end,
      std::string const& description,
      std::enable_if_t<std::is_convertible_v<decltype(std::declval<IT>().
                                                      operator*()),
                                             art::InputTag>>* dummy
      [[maybe_unused]] = nullptr)
      : tags_{begin, end}, description_{description}
    {}

    std::vector<art::InputTag> const&
    wantedTags() const
    {
      return tags_;
    }

  private:
    bool
    doMatch(art::BranchDescription const& p) const override
    {
      return std::any_of(
        std::cbegin(tags_), std::cend(tags_), [&p](art::InputTag const& tag) {
          return (tag.label().empty() || p.moduleLabel() == tag.label()) &&
                 (tag.instance().empty() ||
                  p.productInstanceName() == tag.instance()) &&
                 (tag.process().empty() || p.processName() == tag.process());
        });
    }

    std::string
    doPrint(std::string const& indent) const override
    {
      return indent + description_;
    }

    std::vector<art::InputTag> const tags_;
    std::string description_;
  };

  //----------------------------------------------------------
  // AndHelper template.
  // Used to form expressions involving && between other selectors.
  //----------------------------------------------------------

  template <typename A, typename B>
  class AndHelper : public SelectorBase {
  public:
    AndHelper(A const& a, B const& b) : a_{a}, b_{b} {}

  private:
    bool
    doMatch(BranchDescription const& p) const override
    {
      return a_.match(p) && b_.match(p);
    }

    std::string
    doPrint(std::string const& indent) const override
    {
      return a_.print(indent) + '\n' + b_.print(indent);
    }

    A a_;
    B b_;
  };

  template <is_selector A, is_selector B>
  AndHelper<A, B> operator&&(A const& a, B const& b)
  {
    return AndHelper<A, B>{a, b};
  }

  //----------------------------------------------------------
  // OrHelper template.
  // Used to form expressions involving || between other selectors.
  //----------------------------------------------------------

  template <typename A, typename B>
  class OrHelper : public SelectorBase {
  public:
    OrHelper(A const& a, B const& b) : a_{a}, b_{b} {}

  private:
    bool
    doMatch(BranchDescription const& p) const override
    {
      return a_.match(p) || b_.match(p);
    }

    std::string
    doPrint(std::string const& indent) const override
    {
      std::string result{indent + "[\n"};
      result += indent + a_.print(indent) + '\n';
      result += indent + indent + indent + "or\n";
      result += indent + b_.print(indent) + '\n';
      result += indent + ']';
      return result;
    }

    A a_;
    B b_;
  };

  template <is_selector A, is_selector B>
  OrHelper<A, B>
  operator||(A const& a, B const& b)
  {
    return OrHelper<A, B>{a, b};
  }

  //----------------------------------------------------------
  // NotHelper template.
  // Used to form expressions involving ! acting on a selector.
  //----------------------------------------------------------

  template <typename A>
  class NotHelper : public SelectorBase {
  public:
    explicit NotHelper(A const& a) : a_{a} {}

  private:
    bool
    doMatch(BranchDescription const& p) const override
    {
      return !a_.match(p);
    }

    std::string
    doPrint(std::string const& indent) const override
    {
      std::string result{indent + "Not [\n"};
      result += indent + a_.print(indent) + '\n';
      result += indent + ']';
      return result;
    }
    A a_;
  };

  template <is_selector A>
  NotHelper<A>
  operator!(A const& a)
  {
    return NotHelper<A>{a};
  }

  //----------------------------------------------------------
  // ComposedSelectorWrapper template
  // Used to hold an expression formed from the various helpers.
  //----------------------------------------------------------

  template <typename T>
  class ComposedSelectorWrapper : public SelectorBase {
  public:
    using wrapped_type = T;
    explicit ComposedSelectorWrapper(T const& t) : expression_{t} {}

  private:
    bool
    doMatch(BranchDescription const& p) const override
    {
      return expression_.match(p);
    }

    std::string
    doPrint(std::string const& indent) const override
    {
      return expression_.print(indent);
    }

    wrapped_type expression_;
  };

  //----------------------------------------------------------
  // Selector
  //----------------------------------------------------------

  class Selector : public SelectorBase {
  public:
    template <typename T>
    explicit Selector(T const& expression)
      : sel_{new ComposedSelectorWrapper<T>{expression}}
    {}

  private:
    bool doMatch(BranchDescription const& p) const override;
    std::string doPrint(std::string const& indent) const override;

    std::shared_ptr<SelectorBase> sel_;
  };

} // namespace art

#endif /* art_Framework_Principal_Selector_h */

// Local Variables:
// mode: c++
// End:
