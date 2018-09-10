#ifndef art_Framework_Principal_Selector_h
#define art_Framework_Principal_Selector_h
// vim: set sw=2 expandtab :

/*----------------------------------------------------------------------

Classes for all "selector" objects, used to select
EDProducts based on information in the associated Provenance.

Developers who make their own Selector class should inherit
from SelectorBase.

Users can use the classes defined below

  ModuleLabelSelector
  ProcessNameSelector
  ProductInstanceNameSelector

Users can also use the class Selector, which can be constructed given a
logical expression formed from any other selectors, combined with &&
(the AND operator), || (the OR operator) or ! (the NOT operator).

For example, to select only products produced by a module with label
"mymodule" and made in the process "PROD", one can use:

  Selector s{ModuleLabelSelector("mymodule") &&
             ProcessNameSelector("PROD")};

If a module (EDProducter, EDFilter, EDAnalyzer, or OutputModule) is
to use such a selector, it is best to initialize it directly upon
construction of the module, rather than creating a new Selector instance
for every event.

----------------------------------------------------------------------*/

#include "art/Framework/Principal/SelectorBase.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "cetlib/value_ptr.h"

#include <string>
#include <type_traits>

namespace art {

  template <class A, class B>
  std::enable_if_t<std::is_base_of_v<art::SelectorBase, A> &&
                     std::is_base_of_v<art::SelectorBase, B>,
                   art::AndHelper<A, B>>
  operator&&(A const& a, B const& b);

  template <class A, class B>
  std::enable_if_t<std::is_base_of_v<art::SelectorBase, A> &&
                     std::is_base_of_v<art::SelectorBase, B>,
                   art::OrHelper<A, B>>
  operator||(A const& a, B const& b);

  template <class A>
  std::enable_if_t<std::is_base_of_v<art::SelectorBase, A>, art::NotHelper<A>>
  operator!(A const& a);
} // namespace art

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

class art::ProcessNameSelector : public art::SelectorBase {
public:
  ProcessNameSelector(std::string const& pn)
    : pn_{pn.empty() ? std::string{"*"} : pn}
  {}

  ProcessNameSelector*
  clone() const override
  {
    return new ProcessNameSelector{*this};
  }

  std::string const&
  name() const
  {
    return pn_;
  }

private:
  bool
  doMatch(BranchDescription const& p) const override
  {
    return (pn_ == "*") || (p.processName() == pn_);
  }

  std::string pn_;
};

//------------------------------------------------------------------
// Class ProductInstanceNameSelector.
// Selects EDProducts based upon product instance name.
//------------------------------------------------------------------

class art::ProductInstanceNameSelector : public art::SelectorBase {
public:
  ProductInstanceNameSelector(std::string const& pin) : pin_{pin} {}

  ProductInstanceNameSelector*
  clone() const override
  {
    return new ProductInstanceNameSelector{*this};
  }

private:
  bool
  doMatch(BranchDescription const& p) const override
  {
    return p.productInstanceName() == pin_;
  }

  std::string pin_;
};

//------------------------------------------------------------------
// Class ModuleLabelSelector.
// Selects EDProducts based upon module label.
//------------------------------------------------------------------

class art::ModuleLabelSelector : public art::SelectorBase {
public:
  ModuleLabelSelector(std::string const& label) : label_{label} {}

  ModuleLabelSelector*
  clone() const override
  {
    return new ModuleLabelSelector{*this};
  }

private:
  bool
  doMatch(BranchDescription const& p) const override
  {
    return p.moduleLabel() == label_;
  }

  std::string label_;
};

//------------------------------------------------------------------
// Class MatchAllSelector.
// Dummy selector whose match function always returns true.
//------------------------------------------------------------------

class art::MatchAllSelector : public art::SelectorBase {
public:
  MatchAllSelector*
  clone() const override
  {
    return new MatchAllSelector;
  }

private:
  bool
  doMatch(BranchDescription const&) const override
  {
    return true;
  }
};

//----------------------------------------------------------
// AndHelper template.
// Used to form expressions involving && between other selectors.
//----------------------------------------------------------

template <class A, class B>
class art::AndHelper : public SelectorBase {
public:
  AndHelper(A const& a, B const& b) : a_{a}, b_{b} {}

  AndHelper*
  clone() const override
  {
    return new AndHelper{*this};
  }

private:
  bool
  doMatch(BranchDescription const& p) const override
  {
    return a_.match(p) && b_.match(p);
  }

  A a_;
  B b_;
};

template <class A, class B>
std::enable_if_t<std::is_base_of_v<art::SelectorBase, A> &&
                   std::is_base_of_v<art::SelectorBase, B>,
                 art::AndHelper<A, B>>
art::operator&&(A const& a, B const& b)
{
  return art::AndHelper<A, B>(a, b);
}

//----------------------------------------------------------
// OrHelper template.
// Used to form expressions involving || between other selectors.
//----------------------------------------------------------

template <class A, class B>
class art::OrHelper : public art::SelectorBase {
public:
  OrHelper(A const& a, B const& b) : a_{a}, b_{b} {}

  OrHelper*
  clone() const override
  {
    return new OrHelper{*this};
  }

private:
  bool
  doMatch(BranchDescription const& p) const override
  {
    return a_.match(p) || b_.match(p);
  }

  A a_;
  B b_;
};

template <class A, class B>
std::enable_if_t<std::is_base_of_v<art::SelectorBase, A> &&
                   std::is_base_of_v<art::SelectorBase, B>,
                 art::OrHelper<A, B>>
art::operator||(A const& a, B const& b)
{
  return art::OrHelper<A, B>(a, b);
}

//----------------------------------------------------------
// NotHelper template.
// Used to form expressions involving ! acting on a selector.
//----------------------------------------------------------

template <class A>
class art::NotHelper : public art::SelectorBase {
public:
  explicit NotHelper(A const& a) : a_{a} {}
  NotHelper*
  clone() const override
  {
    return new NotHelper{*this};
  }

private:
  bool
  doMatch(BranchDescription const& p) const override
  {
    return !a_.match(p);
  }

  A a_;
};

template <class A>
std::enable_if_t<std::is_base_of_v<art::SelectorBase, A>, art::NotHelper<A>>
  art::operator!(A const& a)
{
  return art::NotHelper<A>(a);
}

//----------------------------------------------------------
// ComposedSelectorWrapper template
// Used to hold an expression formed from the various helpers.
//----------------------------------------------------------

template <class T>
class art::ComposedSelectorWrapper : public art::SelectorBase {
public:
  using wrapped_type = T;
  explicit ComposedSelectorWrapper(T const& t) : expression_{t} {}

  ComposedSelectorWrapper<T>*
  clone() const override
  {
    return new ComposedSelectorWrapper<T>{*this};
  }

private:
  bool
  doMatch(BranchDescription const& p) const override
  {
    return expression_.match(p);
  }

  wrapped_type expression_;
};

//----------------------------------------------------------
// Selector
//----------------------------------------------------------

class art::Selector : public art::SelectorBase {
public:
  template <class T>
  Selector(T const& expression);
  Selector(Selector const& other);
  Selector& operator=(Selector const& other) &;
  void swap(Selector& other);
  virtual ~Selector();
  Selector* clone() const override;

private:
  bool doMatch(BranchDescription const& p) const override;

  cet::value_ptr<SelectorBase> sel_;
};

template <class T>
art::Selector::Selector(T const& expression)
  : sel_{new ComposedSelectorWrapper<T>(expression)}
{}

#endif /* art_Framework_Principal_Selector_h */

// Local Variables:
// mode: c++
// End:
